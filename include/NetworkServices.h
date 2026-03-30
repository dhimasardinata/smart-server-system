#pragma once

#include "AccessController.h"
#include "Config.h"
#include "GoogleSheetsClient.h"
#include "Sensors.h"
#include "WiFiHandler.h"

#include <Arduino.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <deque>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

class NetworkServices {
 public:
  NetworkServices();

  void begin(ConfigManager* config, WiFiManager* wifi, SensorManager* sensors,
             AccessController* access);
  void update(const SensorData& data, bool fan1On, bool fan2On, bool warning,
              bool solenoidOn, bool alertOn, const char* alertState);
  void logAccessEvent(const AccessEvent& event);

 private:
  AsyncWebServer _server;
  ConfigManager* _config = nullptr;
  WiFiManager* _wifi = nullptr;
  SensorManager* _sensors = nullptr;
  AccessController* _access = nullptr;

  GoogleSheetsClient _googleSheets;

  SensorData _cachedData{};
  bool _cachedFan1On = false;
  bool _cachedFan2On = false;
  bool _cachedWarning = false;
  bool _cachedSolenoidOn = false;
  bool _cachedAlertOn = false;
  const char* _cachedAlertState = "IDLE";
  bool _pendingRestart = false;
  const char* _restartReason = "pending operation";
  unsigned long _restartAtMs = 0;

  unsigned long _lastTelemetryEnqueueMs = 0;
  unsigned long _lastSendEpoch = 0;

  std::deque<TelemetryLogPayload> _telemetryQueue;
  std::deque<AccessLogPayload> _accessQueue;
  unsigned long _nextAttemptMs = 0;
  uint8_t _retryCount = 0;
  bool _forceFlushRequested = false;
  SemaphoreHandle_t _queueMutex = nullptr;
  TaskHandle_t _uploadTask = nullptr;
  bool _asyncUploadEnabled = false;

  void setupRoutes();
  void setupWiFiRoutes();
  void sendCaptiveRedirect(AsyncWebServerRequest* request);
  void notifyUploader();
  bool tryTakeQueueLock(TickType_t waitTicks = portMAX_DELAY);
  size_t telemetryQueueSize();
  size_t accessQueueSize();
  static void uploadTaskEntry(void* context);
  void uploadTaskLoop();

  void handleRoot(AsyncWebServerRequest* request);
  void handleGetState(AsyncWebServerRequest* request);
  void handleGetThermalConfig(AsyncWebServerRequest* request);
  void handleSetThermalConfig(AsyncWebServerRequest* request,
                              JsonVariant& json);
  void handleGetSecurityConfig(AsyncWebServerRequest* request);
  void handleSetSecurityConfig(AsyncWebServerRequest* request,
                               JsonVariant& json);
  void handleGetUsers(AsyncWebServerRequest* request);
  void handleUpsertUser(AsyncWebServerRequest* request, JsonVariant& json);
  void handleDeleteUser(AsyncWebServerRequest* request);
  void handleSendNow(AsyncWebServerRequest* request);
  void handleWiFiScan(AsyncWebServerRequest* request);
  void handleWiFiConnect(AsyncWebServerRequest* request, JsonVariant& json);
  void handleFormatFlash(AsyncWebServerRequest* request);
  void handleOtaUploadResponse(AsyncWebServerRequest* request);
  void handleOtaUploadChunk(AsyncWebServerRequest* request,
                            const String& filename, size_t index,
                            uint8_t* data, size_t len, bool final);

  void enqueueTelemetry();
  String doorState() const;
  String makeTimestampIso8601() const;

  void enqueueTelemetryPayload(const TelemetryLogPayload& payload);
  void enqueueAccessPayload(const AccessLogPayload& payload);
  void flushQueueTick();
  bool sendOne();
  void backoff();
  void scheduleRestart(const char* reason, unsigned long delayMs = 750);
};
