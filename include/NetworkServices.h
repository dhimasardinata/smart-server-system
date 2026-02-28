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

class NetworkServices {
 public:
  NetworkServices();

  void begin(ConfigManager* config, WiFiManager* wifi, SensorManager* sensors,
             AccessController* access);
  void update(const SensorData& data, bool fan1On, bool fan2On, bool warning,
              bool solenoidOn);
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

  unsigned long _lastTelemetryEnqueueMs = 0;
  unsigned long _lastSendEpoch = 0;

  std::deque<TelemetryLogPayload> _telemetryQueue;
  std::deque<AccessLogPayload> _accessQueue;
  unsigned long _nextAttemptMs = 0;
  uint8_t _retryCount = 0;

  void setupRoutes();
  void setupWiFiRoutes();

  void handleRoot(AsyncWebServerRequest* request);
  void handleGetState(AsyncWebServerRequest* request);
  void handleGetThermalConfig(AsyncWebServerRequest* request);
  void handleSetThermalConfig(AsyncWebServerRequest* request, JsonVariant& json);
  void handleGetSecurityConfig(AsyncWebServerRequest* request);
  void handleSetSecurityConfig(AsyncWebServerRequest* request,
                               JsonVariant& json);
  void handleGetUsers(AsyncWebServerRequest* request);
  void handleUpsertUser(AsyncWebServerRequest* request, JsonVariant& json);
  void handleDeleteUser(AsyncWebServerRequest* request);
  void handleSendNow(AsyncWebServerRequest* request);
  void handleWiFiScan(AsyncWebServerRequest* request);
  void handleWiFiConnect(AsyncWebServerRequest* request, JsonVariant& json);

  void enqueueTelemetry();
  String doorState() const;
  String makeTimestampIso8601() const;

  void enqueueTelemetryPayload(const TelemetryLogPayload& payload);
  void enqueueAccessPayload(const AccessLogPayload& payload);
  void flushQueueTick();
  bool flushNow(uint16_t maxItems);
  bool sendOne();
  void backoff();
};
