#include "NetworkServices.h"

#include "WebPage.h"

#include <time.h>

namespace {
constexpr uint16_t MAX_QUEUE_SIZE = 300;
}

NetworkServices::NetworkServices() : _server(80) {}

void NetworkServices::begin(ConfigManager* config, WiFiManager* wifi,
                            SensorManager* sensors, AccessController* access) {
  _config = config;
  _wifi = wifi;
  _sensors = sensors;
  _access = access;

  _googleSheets.begin(_config->data.googleScriptUrl);
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  setupRoutes();
  setupWiFiRoutes();
  _server.begin();
}

void NetworkServices::update(const SensorData& data, bool fan1On, bool fan2On,
                             bool warning, bool solenoidOn) {
  _cachedData = data;
  _cachedFan1On = fan1On;
  _cachedFan2On = fan2On;
  _cachedWarning = warning;
  _cachedSolenoidOn = solenoidOn;

  if (_wifi->isConnected()) {
    if (millis() - _lastTelemetryEnqueueMs >=
        (_config->data.cloudSendIntervalSec * 1000UL)) {
      _lastTelemetryEnqueueMs = millis();
      enqueueTelemetry();
      _lastSendEpoch = static_cast<unsigned long>(time(nullptr));
    }
    flushQueueTick();
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

void NetworkServices::enqueueTelemetryPayload(const TelemetryLogPayload& payload) {
  _telemetryQueue.push_back(payload);
  if (_telemetryQueue.size() > MAX_QUEUE_SIZE) _telemetryQueue.pop_front();
}

void NetworkServices::enqueueAccessPayload(const AccessLogPayload& payload) {
  _accessQueue.push_back(payload);
  if (_accessQueue.size() > MAX_QUEUE_SIZE) _accessQueue.pop_front();
}

void NetworkServices::backoff() {
  _retryCount = min<uint8_t>(_retryCount + 1, 6);
  const unsigned long delayMs = 1000UL << _retryCount;
  _nextAttemptMs = millis() + min(delayMs, 60000UL);
}

bool NetworkServices::sendOne() {
  if (!_googleSheets.isConfigured()) return false;

  bool ok = false;
  if (!_accessQueue.empty()) {
    ok = _googleSheets.sendAccess(_accessQueue.front());
    if (ok) _accessQueue.pop_front();
  } else if (!_telemetryQueue.empty()) {
    ok = _googleSheets.sendTelemetry(_telemetryQueue.front());
    if (ok) _telemetryQueue.pop_front();
  } else {
    return true;
  }

  if (ok) {
    _retryCount = 0;
    _nextAttemptMs = millis();
    return true;
  }

  backoff();
  return false;
}

void NetworkServices::flushQueueTick() {
  if (millis() < _nextAttemptMs) return;
  sendOne();
}

bool NetworkServices::flushNow(uint16_t maxItems) {
  bool allOk = true;
  for (uint16_t i = 0; i < maxItems; ++i) {
    if (_accessQueue.empty() && _telemetryQueue.empty()) break;
    if (!sendOne()) {
      allOk = false;
      break;
    }
  }
  return allOk;
}

String NetworkServices::doorState() const {
  return _cachedSolenoidOn ? "UNLOCKING" : "LOCKED";
}

String NetworkServices::makeTimestampIso8601() const {
  const time_t now = time(nullptr);
  if (now <= 0) return String(millis());

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buf);
}

void NetworkServices::enqueueTelemetry() {
  if (!_cachedData.valid) return;

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
  enqueueTelemetryPayload(payload);
}

void NetworkServices::setupRoutes() {
  _server.on("/", HTTP_GET,
             [this](AsyncWebServerRequest* request) { handleRoot(request); });

  _server.on("/setup", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", WebPage::SETUP_HTML);
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

  _server.onNotFound([this](AsyncWebServerRequest* request) {
    if (request->method() == HTTP_DELETE &&
        request->url().startsWith("/api/users/")) {
      handleDeleteUser(request);
      return;
    }
    request->send(404, "application/json", "{\"error\":\"Not found\"}");
  });
}

void NetworkServices::setupWiFiRoutes() {
  _server.on("/api/wifi/scan", HTTP_GET,
             [this](AsyncWebServerRequest* request) {
               handleWiFiScan(request);
             });

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

void NetworkServices::handleGetState(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["temperature"] = _cachedData.temperature;
  doc["humidity"] = _cachedData.humidity;
  doc["valid"] = _cachedData.valid;
  doc["fan1On"] = _cachedFan1On;
  doc["fan2On"] = _cachedFan2On;
  doc["alarm"] = _cachedWarning;
  doc["doorState"] = doorState();
  doc["solenoidOn"] = _cachedSolenoidOn;
  doc["lockoutActive"] = _access->isLockoutActive();
  doc["lockoutRemainingSec"] = _access->lockoutRemainingSec();
  doc["failedAttempts"] = _access->failedAttempts();
  doc["accessMessage"] = _access->lastMessage();
  doc["queueTelemetry"] = _telemetryQueue.size();
  doc["queueAccess"] = _accessQueue.size();
  doc["wifiConnected"] = _wifi->isConnected();
  doc["ssid"] = _wifi->getSSID();
  doc["rssi"] = _wifi->getRSSI();
  doc["deviceId"] = _config->data.deviceId;
  doc["lastSend"] = _lastSendEpoch;

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void NetworkServices::handleGetThermalConfig(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["warnThreshold"] = _config->data.warnThresholdC;
  doc["stage2Threshold"] = _config->data.stage2ThresholdC;
  doc["fan1BaselineOn"] = _config->data.fan1BaselineOn;
  doc["sensorReadIntervalSec"] = _config->data.sensorReadIntervalSec;
  doc["cloudSendIntervalSec"] = _config->data.cloudSendIntervalSec;

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
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

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
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
    if (user.userId.length() == 0) continue;
    JsonObject item = users.add<JsonObject>();
    item["userId"] = user.userId;
    item["displayName"] = user.displayName;
    item["enabled"] = user.enabled;
  }
  doc["count"] = users.size();

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
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
  if (!_config->removeUser(userId)) {
    request->send(404, "application/json", "{\"error\":\"user not found\"}");
    return;
  }
  request->send(200, "application/json", "{\"success\":true}");
}

void NetworkServices::handleSendNow(AsyncWebServerRequest* request) {
  const bool ok = flushNow(50);
  JsonDocument doc;
  doc["success"] = ok;
  doc["queueTelemetry"] = _telemetryQueue.size();
  doc["queueAccess"] = _accessQueue.size();
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void NetworkServices::handleWiFiScan(AsyncWebServerRequest* request) {
  auto networks = _wifi->getScannedNetworks();
  JsonDocument doc;
  JsonArray arr = doc["networks"].to<JsonArray>();
  for (const auto& net : networks) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ssid"] = net.ssid;
    obj["rssi"] = net.rssi;
    obj["open"] = net.open;
    obj["saved"] = net.saved;
  }
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
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

  _config->addWiFi(ssid, password);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
  }

  JsonDocument doc;
  if (WiFi.status() == WL_CONNECTED) {
    doc["success"] = true;
    doc["ip"] = WiFi.localIP().toString();
  } else {
    doc["success"] = false;
    doc["error"] = "Connection failed";
  }
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}
