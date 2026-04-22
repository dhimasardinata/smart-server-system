#include "NetworkServices.h"

#include "OtaCoordinator.h"
#include "WebPage.h"

#include <Esp.h>
#include <Update.h>
#include <time.h>

namespace {
constexpr uint16_t MAX_QUEUE_SIZE = 300;
constexpr const char* LOCAL_MDNS_HOST = "monitor-server.local";
constexpr unsigned long MIN_SEND_GAP_MS = 250;
constexpr unsigned long DISCONNECTED_WAIT_MS = 1000;
constexpr unsigned long IDLE_WAIT_MS = 2000;
constexpr uint32_t UPLOAD_TASK_STACK_WORDS = 8192;
constexpr UBaseType_t UPLOAD_TASK_PRIORITY = 1;

struct FirmwareUploadContext {
  bool accepted = false;
  bool success = false;
  bool failed = false;
  int statusCode = 500;
  size_t totalBytes = 0;
  String filename;
  String error;
};

bool isApiPath(const String& url) {
  return url.startsWith("/api/");
}

void sendJson(AsyncWebServerRequest* request, JsonDocument& doc) {
  AsyncResponseStream* response =
      request->beginResponseStream("application/json");
  serializeJson(doc, *response);
  request->send(response);
}

size_t requestFirmwareSize(AsyncWebServerRequest* request) {
  if (request == nullptr || !request->hasHeader("X-Firmware-Size")) {
    return UPDATE_SIZE_UNKNOWN;
  }

  const AsyncWebHeader* header = request->getHeader("X-Firmware-Size");
  if (header == nullptr) {
    return UPDATE_SIZE_UNKNOWN;
  }

  const unsigned long parsed = strtoul(header->value().c_str(), nullptr, 10);
  return parsed > 0 ? parsed : UPDATE_SIZE_UNKNOWN;
}

FirmwareUploadContext* uploadContext(AsyncWebServerRequest* request) {
  if (request == nullptr) {
    return nullptr;
  }
  return static_cast<FirmwareUploadContext*>(request->_tempObject);
}

void clearUploadContext(AsyncWebServerRequest* request) {
  FirmwareUploadContext* context = uploadContext(request);
  delete context;
  if (request != nullptr) {
    request->_tempObject = nullptr;
  }
}
}  // namespace

NetworkServices::NetworkServices() : _server(80) {}

void NetworkServices::begin(ConfigManager* config, WiFiManager* wifi,
                            SensorManager* sensors, AccessController* access) {
  _config = config;
  _wifi = wifi;
  _sensors = sensors;
  _access = access;
  OtaCoordinator::instance().begin();

  _googleSheets.begin(_config->data.googleScriptUrl);
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  _queueMutex = xSemaphoreCreateMutex();
  if (_queueMutex == nullptr) {
    Serial.println(
        F("NetworkServices: queue mutex init failed, using sync upload"));
  } else if (xTaskCreate(NetworkServices::uploadTaskEntry, "cloud_upload",
                         UPLOAD_TASK_STACK_WORDS, this, UPLOAD_TASK_PRIORITY,
                         &_uploadTask) != pdPASS) {
    Serial.println(
        F("NetworkServices: upload task init failed, using sync upload"));
  } else {
    _asyncUploadEnabled = true;
  }

  setupRoutes();
  setupWiFiRoutes();
  _server.begin();
}

void NetworkServices::update(const SensorData& data, bool fan1On, bool fan2On,
                             bool warning, bool solenoidOn, bool alertOn,
                             const char* alertState) {
  if (_pendingRestart && millis() >= _restartAtMs) {
    Serial.printf("Restarting after %s\n", _restartReason);
    delay(100);
    ESP.restart();
    return;
  }

  _cachedData = data;
  _cachedFan1On = fan1On;
  _cachedFan2On = fan2On;
  _cachedWarning = warning;
  _cachedSolenoidOn = solenoidOn;
  _cachedAlertOn = alertOn;
  _cachedAlertState = alertState != nullptr ? alertState : "IDLE";

  const OtaCoordinator::Snapshot otaState = OtaCoordinator::instance().snapshot();
  if (otaState.busy) {
    return;
  }

  if (_wifi->isConnected()) {
    if (millis() - _lastTelemetryEnqueueMs >=
        (_config->data.cloudSendIntervalSec * 1000UL)) {
      _lastTelemetryEnqueueMs = millis();
      enqueueTelemetry();
    }
    if (!_asyncUploadEnabled) {
      flushQueueTick();
    }
  }
}

void NetworkServices::logAccessEvent(const AccessEvent& event) {
  AccessLogPayload payload;
  payload.timestamp = makeTimestampIso8601();
  payload.deviceId = _config->data.deviceId;
  payload.userId = event.userId.length() > 0 ? event.userId : "unknown";
  payload.displayName =
      event.displayName.length() > 0 ? event.displayName : "Unknown";
  payload.result = event.result;
  payload.reason = event.reason;
  payload.failedCount = event.failedCount;
  payload.lockoutUntil = event.lockoutUntilEpoch;
  payload.doorState = doorState();
  enqueueAccessPayload(payload);
}

void NetworkServices::enqueueTelemetryPayload(
    const TelemetryLogPayload& payload) {
  if (tryTakeQueueLock()) {
    _telemetryQueue.push_back(payload);
    if (_telemetryQueue.size() > MAX_QUEUE_SIZE)
      _telemetryQueue.pop_front();
    if (_queueMutex != nullptr) {
      xSemaphoreGive(_queueMutex);
    }
  }
  notifyUploader();
}

void NetworkServices::notifyUploader() {
  if (_asyncUploadEnabled && _uploadTask != nullptr) {
    xTaskNotifyGive(_uploadTask);
  }
}

bool NetworkServices::tryTakeQueueLock(TickType_t waitTicks) {
  if (_queueMutex == nullptr) {
    return true;
  }
  return xSemaphoreTake(_queueMutex, waitTicks) == pdTRUE;
}

size_t NetworkServices::telemetryQueueSize() {
  if (!tryTakeQueueLock(pdMS_TO_TICKS(20))) {
    return _telemetryQueue.size();
  }
  const size_t size = _telemetryQueue.size();
  if (_queueMutex != nullptr) {
    xSemaphoreGive(_queueMutex);
  }
  return size;
}

size_t NetworkServices::accessQueueSize() {
  if (!tryTakeQueueLock(pdMS_TO_TICKS(20))) {
    return _accessQueue.size();
  }
  const size_t size = _accessQueue.size();
  if (_queueMutex != nullptr) {
    xSemaphoreGive(_queueMutex);
  }
  return size;
}

void NetworkServices::uploadTaskEntry(void* context) {
  static_cast<NetworkServices*>(context)->uploadTaskLoop();
}

void NetworkServices::uploadTaskLoop() {
  for (;;) {
    bool hasPending = false;
    bool shouldSend = false;
    bool forceFlush = false;
    unsigned long nextAttemptMs = 0;
    const bool connected = (_wifi != nullptr) && _wifi->isConnected();

    if (tryTakeQueueLock(pdMS_TO_TICKS(50))) {
      hasPending = !_accessQueue.empty() || !_telemetryQueue.empty();
      forceFlush = _forceFlushRequested;
      nextAttemptMs = _nextAttemptMs;
      if (_queueMutex != nullptr) {
        xSemaphoreGive(_queueMutex);
      }
    }

    const unsigned long now = millis();
    unsigned long waitMs = IDLE_WAIT_MS;
    const OtaCoordinator::Snapshot otaState = OtaCoordinator::instance().snapshot();

    if (otaState.busy) {
      waitMs = IDLE_WAIT_MS;
    } else if (!connected) {
      waitMs = hasPending ? DISCONNECTED_WAIT_MS : IDLE_WAIT_MS;
    } else if (!hasPending) {
      waitMs = IDLE_WAIT_MS;
    } else if (forceFlush || now >= nextAttemptMs) {
      shouldSend = true;
    } else {
      waitMs = min(nextAttemptMs - now, DISCONNECTED_WAIT_MS);
    }

    if (shouldSend) {
      sendOne();
      continue;
    }

    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(max(1UL, waitMs)));
  }
}

void NetworkServices::enqueueAccessPayload(const AccessLogPayload& payload) {
  if (tryTakeQueueLock()) {
    _accessQueue.push_back(payload);
    if (_accessQueue.size() > MAX_QUEUE_SIZE)
      _accessQueue.pop_front();
    if (_queueMutex != nullptr) {
      xSemaphoreGive(_queueMutex);
    }
  }
  notifyUploader();
}

void NetworkServices::backoff() {
  _retryCount = min<uint8_t>(_retryCount + 1, 6);
  const unsigned long delayMs = 1000UL << _retryCount;
  _nextAttemptMs = millis() + min(delayMs, 60000UL);
  _forceFlushRequested = false;
}

bool NetworkServices::sendOne() {
  if (!_googleSheets.isConfigured()) {
    if (tryTakeQueueLock()) {
      _nextAttemptMs = millis() + 10000UL;
      _forceFlushRequested = false;
      if (_queueMutex != nullptr) {
        xSemaphoreGive(_queueMutex);
      }
    }
    return false;
  }

  bool sendAccessPayload = false;
  AccessLogPayload accessPayload;
  TelemetryLogPayload telemetryPayload;

  if (!tryTakeQueueLock()) {
    return false;
  }

  if (!_accessQueue.empty()) {
    sendAccessPayload = true;
    accessPayload = _accessQueue.front();
  } else if (!_telemetryQueue.empty()) {
    telemetryPayload = _telemetryQueue.front();
  } else {
    _forceFlushRequested = false;
    if (_queueMutex != nullptr) {
      xSemaphoreGive(_queueMutex);
    }
    return true;
  }

  if (_queueMutex != nullptr) {
    xSemaphoreGive(_queueMutex);
  }

  const bool ok = sendAccessPayload
                      ? _googleSheets.sendAccess(accessPayload)
                      : _googleSheets.sendTelemetry(telemetryPayload);

  if (!tryTakeQueueLock()) {
    return ok;
  }

  if (ok) {
    if (sendAccessPayload) {
      if (!_accessQueue.empty()) {
        _accessQueue.pop_front();
      }
    } else if (!_telemetryQueue.empty()) {
      _telemetryQueue.pop_front();
    }
    _retryCount = 0;
    _nextAttemptMs = millis() + MIN_SEND_GAP_MS;
    const bool hasMore = !_accessQueue.empty() || !_telemetryQueue.empty();
    _forceFlushRequested = _forceFlushRequested && hasMore;
    _lastSendEpoch = static_cast<unsigned long>(time(nullptr));
    if (_queueMutex != nullptr) {
      xSemaphoreGive(_queueMutex);
    }
    if (_asyncUploadEnabled && hasMore) {
      notifyUploader();
    }
    return true;
  }

  backoff();
  if (_queueMutex != nullptr) {
    xSemaphoreGive(_queueMutex);
  }
  return false;
}

void NetworkServices::flushQueueTick() {
  if (_asyncUploadEnabled) {
    notifyUploader();
    return;
  }
  if (!tryTakeQueueLock(pdMS_TO_TICKS(20))) {
    return;
  }
  const bool hasPending = !_accessQueue.empty() || !_telemetryQueue.empty();
  const bool shouldSend =
      hasPending && (_forceFlushRequested || millis() >= _nextAttemptMs);
  if (!hasPending) {
    _forceFlushRequested = false;
  }
  if (_queueMutex != nullptr) {
    xSemaphoreGive(_queueMutex);
  }
  if (shouldSend) {
    sendOne();
  }
}

String NetworkServices::doorState() const {
  return _cachedSolenoidOn ? "UNLOCKING" : "LOCKED";
}

String NetworkServices::makeTimestampIso8601() const {
  const time_t now = time(nullptr);
  if (now <= 0)
    return String(millis());

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buf);
}

void NetworkServices::enqueueTelemetry() {
  if (!_cachedData.valid)
    return;

  TelemetryLogPayload payload;
  payload.timestamp = makeTimestampIso8601();
  payload.deviceId = _config->data.deviceId;
  payload.temperatureC = _cachedData.temperature;
  payload.humidityPct = _cachedData.humidity;
  payload.fan1On = _cachedFan1On;
  payload.fan2On = _cachedFan2On;
  payload.alarmState = _cachedWarning;
  payload.doorState = doorState();
  payload.wifiRssi = _wifi->getRSSI();
  payload.warnThreshold = _config->data.warnThresholdC;
  payload.stage2Threshold = _config->data.stage2ThresholdC;
  payload.warnHumThreshold = _config->data.warnHumPct;
  payload.stage2HumThreshold = _config->data.stage2HumPct;
  enqueueTelemetryPayload(payload);
}

void NetworkServices::setupRoutes() {
  _server.on("/", HTTP_GET,
             [this](AsyncWebServerRequest* request) { handleRoot(request); });

  _server.on("/setup", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", WebPage::SETUP_HTML);
  });

  _server.on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });
  _server.on("/gen_204", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });
  _server.on(
      "/hotspot-detect.html", HTTP_GET,
      [this](AsyncWebServerRequest* request) { sendCaptiveRedirect(request); });
  _server.on(
      "/connecttest.txt", HTTP_GET,
      [this](AsyncWebServerRequest* request) { sendCaptiveRedirect(request); });
  _server.on("/redirect", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });
  _server.on("/fwlink", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });
  _server.on(
      "/canonical.html", HTTP_GET,
      [this](AsyncWebServerRequest* request) { sendCaptiveRedirect(request); });
  _server.on("/success.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });
  _server.on("/ncsi.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
    sendCaptiveRedirect(request);
  });

  _server.on("/api/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetState(request);
  });

  _server.on("/api/config/thermal", HTTP_GET,
             [this](AsyncWebServerRequest* request) {
               handleGetThermalConfig(request);
             });
  AsyncCallbackJsonWebHandler* thermalConfigHandler =
      new AsyncCallbackJsonWebHandler(
          "/api/config/thermal",
          [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetThermalConfig(request, json);
          });
  _server.addHandler(thermalConfigHandler);

  _server.on("/api/config/security", HTTP_GET,
             [this](AsyncWebServerRequest* request) {
               handleGetSecurityConfig(request);
             });
  AsyncCallbackJsonWebHandler* securityConfigHandler =
      new AsyncCallbackJsonWebHandler(
          "/api/config/security",
          [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetSecurityConfig(request, json);
          });
  _server.addHandler(securityConfigHandler);

  _server.on("/api/users", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleGetUsers(request);
  });
  AsyncCallbackJsonWebHandler* usersHandler = new AsyncCallbackJsonWebHandler(
      "/api/users", [this](AsyncWebServerRequest* request, JsonVariant& json) {
        handleUpsertUser(request, json);
      });
  _server.addHandler(usersHandler);

  _server.on("/api/send", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleSendNow(request);
  });

  _server.on(
      "/api/flash/format", HTTP_POST,
      [this](AsyncWebServerRequest* request) { handleFormatFlash(request); });

  _server.on(
      "/api/ota/upload", HTTP_POST,
      [this](AsyncWebServerRequest* request) { handleOtaUploadResponse(request); },
      [this](AsyncWebServerRequest* request, const String& filename,
             size_t index, uint8_t* data, size_t len, bool final) {
        handleOtaUploadChunk(request, filename, index, data, len, final);
      });

  _server.onNotFound([this](AsyncWebServerRequest* request) {
    if (request->method() == HTTP_DELETE &&
        request->url().startsWith("/api/users/")) {
      handleDeleteUser(request);
      return;
    }

    if (_wifi->isApMode() && request->method() == HTTP_GET &&
        !isApiPath(request->url())) {
      sendCaptiveRedirect(request);
      return;
    }

    request->send(404, "application/json", "{\"error\":\"Not found\"}");
  });
}

void NetworkServices::setupWiFiRoutes() {
  _server.on(
      "/api/wifi/scan", HTTP_GET,
      [this](AsyncWebServerRequest* request) { handleWiFiScan(request); });

  AsyncCallbackJsonWebHandler* wifiHandler = new AsyncCallbackJsonWebHandler(
      "/api/wifi/connect",
      [this](AsyncWebServerRequest* request, JsonVariant& json) {
        handleWiFiConnect(request, json);
      });
  _server.addHandler(wifiHandler);
}

void NetworkServices::handleRoot(AsyncWebServerRequest* request) {
  if (_wifi->isApMode()) {
    request->send(200, "text/html", WebPage::SETUP_HTML);
  } else {
    request->send(200, "text/html", WebPage::DASHBOARD_HTML);
  }
}

void NetworkServices::sendCaptiveRedirect(AsyncWebServerRequest* request) {
  if (!_wifi->isApMode()) {
    handleRoot(request);
    return;
  }

  AsyncWebServerResponse* response = request->beginResponse(302);
  response->addHeader("Location",
                      String("http://") + _wifi->getIP().toString() + "/setup");
  response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  request->send(response);
}

void NetworkServices::handleGetState(AsyncWebServerRequest* request) {
  size_t queueTelemetry = telemetryQueueSize();
  size_t queueAccess = accessQueueSize();
  unsigned long lastSendEpoch = _lastSendEpoch;
  if (tryTakeQueueLock(pdMS_TO_TICKS(20))) {
    lastSendEpoch = _lastSendEpoch;
    if (_queueMutex != nullptr) {
      xSemaphoreGive(_queueMutex);
    }
  }

  JsonDocument doc;
  const OtaCoordinator::Snapshot otaState = OtaCoordinator::instance().snapshot();
  doc["temperature"] = _cachedData.temperature;
  doc["humidity"] = _cachedData.humidity;
  doc["valid"] = _cachedData.valid;
  doc["fan1On"] = _cachedFan1On;
  doc["fan2On"] = _cachedFan2On;
  doc["alarm"] = _cachedWarning;
  doc["doorState"] = doorState();
  doc["solenoidOn"] = _cachedSolenoidOn;
  doc["alertOn"] = _cachedAlertOn;
  doc["alertState"] = _cachedAlertState;
  doc["lockoutActive"] = _access->isLockoutActive();
  doc["lockoutRemainingSec"] = _access->lockoutRemainingSec();
  doc["failedAttempts"] = _access->failedAttempts();
  doc["accessMessage"] = _access->lastMessage();
  doc["queueTelemetry"] = queueTelemetry;
  doc["queueAccess"] = queueAccess;
  doc["wifiConnected"] = _wifi->isConnected();
  doc["wifiState"] = _wifi->stateName();
  doc["apMode"] = _wifi->isApMode();
  doc["ssid"] = _wifi->getSSID();
  doc["ip"] = _wifi->getIP().toString();
  doc["rssi"] = _wifi->getRSSI();
  doc["mdns"] = _wifi->isConnected() ? LOCAL_MDNS_HOST : "";
  doc["deviceId"] = _config->data.deviceId;
  doc["lastSend"] = lastSendEpoch;
  doc["scanPending"] = _wifi->isScanPending();
  doc["scanAgeMs"] = _wifi->lastScanAgeMs();
  doc["otaBusy"] = otaState.busy;
  doc["otaMode"] = OtaCoordinator::instance().modeName();
  doc["otaProgress"] = otaState.progress;
  doc["otaMessage"] = otaState.message;
  doc["restartPending"] = _pendingRestart;
  sendJson(request, doc);
}

void NetworkServices::handleGetThermalConfig(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["warnThreshold"] = _config->data.warnThresholdC;
  doc["stage2Threshold"] = _config->data.stage2ThresholdC;
  doc["warnHumPct"] = _config->data.warnHumPct;
  doc["stage2HumPct"] = _config->data.stage2HumPct;
  doc["fan1BaselineOn"] = _config->data.fan1BaselineOn;
  doc["sensorReadIntervalSec"] = _config->data.sensorReadIntervalSec;
  doc["cloudSendIntervalSec"] = _config->data.cloudSendIntervalSec;
  sendJson(request, doc);
}

void NetworkServices::handleSetThermalConfig(AsyncWebServerRequest* request,
                                             JsonVariant& json) {
  JsonObject obj = json.as<JsonObject>();
  if (obj["warnThreshold"].is<float>()) {
    _config->data.warnThresholdC = obj["warnThreshold"].as<float>();
  }
  if (obj["stage2Threshold"].is<float>()) {
    _config->data.stage2ThresholdC = obj["stage2Threshold"].as<float>();
  }
  if (obj["warnHumPct"].is<float>()) {
    _config->data.warnHumPct = obj["warnHumPct"].as<float>();
  }
  if (obj["stage2HumPct"].is<float>()) {
    _config->data.stage2HumPct = obj["stage2HumPct"].as<float>();
  }
  if (obj["fan1BaselineOn"].is<bool>()) {
    _config->data.fan1BaselineOn = obj["fan1BaselineOn"].as<bool>();
  }
  if (obj["sensorReadIntervalSec"].is<uint32_t>()) {
    _config->data.sensorReadIntervalSec =
        max<uint32_t>(obj["sensorReadIntervalSec"].as<uint32_t>(), 1);
    _sensors->setReadIntervalMs(_config->data.sensorReadIntervalSec * 1000UL);
  }
  if (obj["cloudSendIntervalSec"].is<uint32_t>()) {
    _config->data.cloudSendIntervalSec =
        max<uint32_t>(obj["cloudSendIntervalSec"].as<uint32_t>(), 10);
  }
  _config->save();
  request->send(200, "application/json", "{\"success\":true}");
}

void NetworkServices::handleGetSecurityConfig(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["maxFail"] = _config->data.maxFailedAttempts;
  doc["lockoutSecs"] = _config->data.keypadLockoutSec;
  doc["unlockSecs"] = _config->data.solenoidUnlockSec;
  doc["deviceId"] = _config->data.deviceId;
  sendJson(request, doc);
}

void NetworkServices::handleSetSecurityConfig(AsyncWebServerRequest* request,
                                              JsonVariant& json) {
  JsonObject obj = json.as<JsonObject>();
  if (obj["maxFail"].is<uint8_t>()) {
    _config->data.maxFailedAttempts =
        max<uint8_t>(obj["maxFail"].as<uint8_t>(), 1);
  }
  if (obj["lockoutSecs"].is<uint32_t>()) {
    _config->data.keypadLockoutSec =
        max<uint32_t>(obj["lockoutSecs"].as<uint32_t>(), 10);
  }
  if (obj["unlockSecs"].is<uint32_t>()) {
    _config->data.solenoidUnlockSec =
        max<uint32_t>(obj["unlockSecs"].as<uint32_t>(), 1);
  }
  if (obj["deviceId"].is<const char*>()) {
    _config->data.deviceId = obj["deviceId"].as<String>();
  }
  _config->save();
  request->send(200, "application/json", "{\"success\":true}");
}

void NetworkServices::handleGetUsers(AsyncWebServerRequest* request) {
  JsonDocument doc;
  JsonArray users = doc["users"].to<JsonArray>();
  for (const auto& user : _config->data.users) {
    if (user.userId.length() == 0)
      continue;
    JsonObject item = users.add<JsonObject>();
    item["userId"] = user.userId;
    item["displayName"] = user.displayName;
    item["enabled"] = user.enabled;
  }
  doc["count"] = users.size();
  sendJson(request, doc);
}

void NetworkServices::handleUpsertUser(AsyncWebServerRequest* request,
                                       JsonVariant& json) {
  JsonObject obj = json.as<JsonObject>();
  const String userId = obj["userId"] | "";
  const String displayName = obj["displayName"] | "";
  const String pin = obj["pin"] | "";
  const bool enabled = obj["enabled"] | true;

  String error;
  if (!_access->upsertUser(userId, displayName, pin, enabled, error)) {
    String body = "{\"error\":\"" + error + "\"}";
    request->send(400, "application/json", body);
    return;
  }

  request->send(200, "application/json", "{\"success\":true}");
}

void NetworkServices::handleDeleteUser(AsyncWebServerRequest* request) {
  const String path = request->url();
  const String userId = path.substring(String("/api/users/").length());
  if (userId.length() == 0) {
    request->send(400, "application/json", "{\"error\":\"userId required\"}");
    return;
  }
  const UserCredential* existing = _config->findUser(userId);
  if (existing == nullptr) {
    request->send(404, "application/json", "{\"error\":\"user not found\"}");
    return;
  }
  if (existing->enabled && _config->getUserCount() <= 1) {
    request->send(400, "application/json",
                  "{\"error\":\"cannot delete last enabled user\"}");
    return;
  }
  if (!_config->removeUser(userId)) {
    request->send(500, "application/json",
                  "{\"error\":\"failed to delete user\"}");
    return;
  }
  request->send(200, "application/json", "{\"success\":true}");
}

void NetworkServices::handleSendNow(AsyncWebServerRequest* request) {
  JsonDocument doc;
  if (!_wifi->isConnected()) {
    doc["success"] = false;
    doc["error"] = "WiFi not connected";
  } else if (!_googleSheets.isConfigured()) {
    doc["success"] = false;
    doc["error"] = "Google Sheets not configured";
  } else {
    if (tryTakeQueueLock()) {
      _forceFlushRequested = true;
      _nextAttemptMs = millis();
      if (_queueMutex != nullptr) {
        xSemaphoreGive(_queueMutex);
      }
    }
    notifyUploader();
    doc["success"] = true;
    doc["pending"] = telemetryQueueSize() > 0 || accessQueueSize() > 0;
  }
  doc["queueTelemetry"] = telemetryQueueSize();
  doc["queueAccess"] = accessQueueSize();
  sendJson(request, doc);
}

void NetworkServices::handleWiFiScan(AsyncWebServerRequest* request) {
  _wifi->requestScanRefresh();
  const auto networks = _wifi->getScannedNetworks();
  JsonDocument doc;
  JsonArray arr = doc["networks"].to<JsonArray>();
  for (const auto& net : networks) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ssid"] = net.ssid;
    obj["rssi"] = net.rssi;
    obj["open"] = net.open;
    obj["saved"] = net.saved;
  }
  doc["pending"] = _wifi->isScanPending();
  doc["scanAgeMs"] = _wifi->lastScanAgeMs();
  sendJson(request, doc);
}

void NetworkServices::handleWiFiConnect(AsyncWebServerRequest* request,
                                        JsonVariant& json) {
  JsonObject obj = json.as<JsonObject>();
  const String ssid = obj["ssid"].as<String>();
  const String password = obj["password"].as<String>();

  if (ssid.length() == 0) {
    request->send(400, "application/json", "{\"error\":\"SSID required\"}");
    return;
  }

  if (!_config->addWiFi(ssid, password)) {
    request->send(500, "application/json",
                  "{\"error\":\"Failed to save WiFi credentials\"}");
    return;
  }

  JsonDocument doc;
  if (_wifi->connectTo(ssid, password)) {
    doc["success"] = true;
    doc["pending"] = true;
    doc["message"] = "Connecting";
    doc["state"] = _wifi->stateName();
  } else {
    doc["success"] = false;
    doc["error"] = "Failed to start WiFi connection";
  }
  sendJson(request, doc);
}

void NetworkServices::handleFormatFlash(AsyncWebServerRequest* request) {
  if (!_config->formatFileSystem()) {
    request->send(500, "application/json",
                  "{\"error\":\"Failed to format flash\"}");
    return;
  }

  scheduleRestart("flash format");
  request->send(
      200, "application/json",
      "{\"success\":true,\"message\":\"Flash formatted. Restarting.\"}");
}

void NetworkServices::handleOtaUploadResponse(AsyncWebServerRequest* request) {
  FirmwareUploadContext* context = uploadContext(request);
  JsonDocument doc;

  if (context == nullptr) {
    doc["success"] = false;
    doc["error"] = "Tidak ada file firmware";
    AsyncResponseStream* response =
        request->beginResponseStream("application/json");
    response->setCode(400);
    serializeJson(doc, *response);
    request->send(response);
    return;
  }

  if (context->failed || !context->success) {
    doc["success"] = false;
    doc["error"] =
        context->error.length() > 0 ? context->error : "Upload firmware gagal";
    AsyncResponseStream* response =
        request->beginResponseStream("application/json");
    response->setCode(context->statusCode);
    serializeJson(doc, *response);
    request->send(response);
    clearUploadContext(request);
    return;
  }

  scheduleRestart("web ota", 1800);
  doc["success"] = true;
  doc["message"] = "Firmware diterima. ESP akan restart.";
  doc["filename"] = context->filename;
  doc["bytes"] = context->totalBytes;
  request->send(200, "application/json",
                "{\"success\":true,\"message\":\"Firmware diterima. ESP akan restart.\"}");
  clearUploadContext(request);
}

void NetworkServices::handleOtaUploadChunk(AsyncWebServerRequest* request,
                                           const String& filename,
                                           size_t index, uint8_t* data,
                                           size_t len, bool final) {
  FirmwareUploadContext* context = uploadContext(request);
  if (index == 0 && context == nullptr) {
    context = new FirmwareUploadContext();
    request->_tempObject = context;
  }
  if (context == nullptr || context->failed) {
    return;
  }

  if (index == 0) {
    context->filename = filename.length() > 0 ? filename : "firmware.bin";
    context->totalBytes = requestFirmwareSize(request);

    if (!context->filename.endsWith(".bin")) {
      context->failed = true;
      context->statusCode = 400;
      context->error = "File firmware harus berekstensi .bin";
      OtaCoordinator::instance().finishWeb(false, context->error);
      return;
    }

    if (!OtaCoordinator::instance().beginWeb(context->totalBytes,
                                             context->filename)) {
      context->failed = true;
      context->statusCode = 409;
      context->error = "OTA lain sedang berjalan";
      return;
    }

    if (!Update.begin(context->totalBytes, U_FLASH)) {
      context->failed = true;
      context->statusCode = 500;
      context->error = String("Gagal memulai update: ") + Update.errorString();
      OtaCoordinator::instance().finishWeb(false, context->error);
      Update.abort();
      return;
    }

    context->accepted = true;
  }

  if (!context->accepted) {
    return;
  }

  if (len > 0 && Update.write(data, len) != len) {
    context->failed = true;
    context->statusCode = 500;
    context->error = String("Gagal menulis firmware: ") + Update.errorString();
    OtaCoordinator::instance().finishWeb(false, context->error);
    Update.abort();
    return;
  }

  const size_t writtenBytes = index + len;
  OtaCoordinator::instance().updateWebProgress(writtenBytes, context->totalBytes);

  if (!final) {
    return;
  }

  if (!Update.end(true)) {
    context->failed = true;
    context->statusCode = 500;
    context->error = String("Gagal menyelesaikan update: ") + Update.errorString();
    OtaCoordinator::instance().finishWeb(false, context->error);
    Update.abort();
    return;
  }

  context->success = true;
  OtaCoordinator::instance().finishWeb(true, "Web OTA selesai. Menunggu restart");
}

void NetworkServices::scheduleRestart(const char* reason, unsigned long delayMs) {
  _pendingRestart = true;
  _restartReason =
      (reason != nullptr && reason[0] != '\0') ? reason : "pending operation";
  _restartAtMs = millis() + delayMs;
}
