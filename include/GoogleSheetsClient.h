#pragma once

#include <Arduino.h>

struct TelemetryLogPayload {
  String timestamp;
  String deviceId;
  float temperatureC = 0.0f;
  float humidityPct = 0.0f;
  bool fan1On = false;
  bool fan2On = false;
  bool alarmState = false;
  String doorState;
  int32_t wifiRssi = 0;
  float warnThreshold = 0.0f;
  float stage2Threshold = 0.0f;
};

struct AccessLogPayload {
  String timestamp;
  String deviceId;
  String userId;
  String displayName;
  String result;
  String reason;
  uint8_t failedCount = 0;
  uint32_t lockoutUntil = 0;
  String doorState;
};

class GoogleSheetsClient {
 public:
  void begin(const String& scriptUrl);

  bool sendTelemetry(const TelemetryLogPayload& payload);
  bool sendAccess(const AccessLogPayload& payload);

  bool isConfigured() const { return _configured; }
  int getLastHttpCode() const { return _lastHttpCode; }
  const String& getLastError() const { return _lastError; }

 private:
  String _scriptUrl;
  bool _configured = false;
  int _lastHttpCode = 0;
  String _lastError;

  bool sendGetRequest(const String& url);
};
