#include "GoogleSheetsClient.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

namespace {
String urlEncode(const String& value) {
  static const char* kHex = "0123456789ABCDEF";
  String out;
  out.reserve(value.length() * 3);

  for (size_t i = 0; i < value.length(); ++i) {
    const uint8_t c = static_cast<uint8_t>(value[i]);
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
        c == '~') {
      out += static_cast<char>(c);
    } else {
      out += '%';
      out += kHex[(c >> 4) & 0x0F];
      out += kHex[c & 0x0F];
    }
  }
  return out;
}
}  // namespace

void GoogleSheetsClient::begin(const String& scriptUrl) {
  _scriptUrl = scriptUrl;
  _configured = scriptUrl.length() > 0 && scriptUrl.startsWith("https://");
}

bool GoogleSheetsClient::sendGetRequest(const String& url) {
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(20000);

  HTTPClient http;
  http.setTimeout(30000);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

  if (!http.begin(client, url)) {
    _lastError = "HTTP begin failed";
    return false;
  }

  _lastHttpCode = http.GET();
  const String response = http.getString();
  http.end();

  if (_lastHttpCode != 200 && _lastHttpCode != 302) {
    _lastError = "HTTP " + String(_lastHttpCode) + ": " + response;
    return false;
  }
  return true;
}

bool GoogleSheetsClient::sendTelemetry(const TelemetryLogPayload& payload) {
  if (!_configured) {
    _lastError = "Google Sheets not configured";
    return false;
  }

  String url = _scriptUrl;
  url += "?sheet=telemetry_logs";
  url += "&timestamp=" + urlEncode(payload.timestamp);
  url += "&device_id=" + urlEncode(payload.deviceId);
  url += "&temperature_c=" + String(payload.temperatureC, 2);
  url += "&humidity_pct=" + String(payload.humidityPct, 2);
  url += "&fan1_on=" + String(payload.fan1On ? "true" : "false");
  url += "&fan2_on=" + String(payload.fan2On ? "true" : "false");
  url += "&alarm_state=" + String(payload.alarmState ? "ALARM" : "NORMAL");
  url += "&door_state=" + urlEncode(payload.doorState);
  url += "&wifi_rssi=" + String(payload.wifiRssi);
  return sendGetRequest(url);
}

bool GoogleSheetsClient::sendAccess(const AccessLogPayload& payload) {
  if (!_configured) {
    _lastError = "Google Sheets not configured";
    return false;
  }

  String url = _scriptUrl;
  url += "?sheet=access_logs";
  url += "&timestamp=" + urlEncode(payload.timestamp);
  url += "&device_id=" + urlEncode(payload.deviceId);
  url += "&user_id=" + urlEncode(payload.userId);
  url += "&display_name=" + urlEncode(payload.displayName);
  url += "&result=" + urlEncode(payload.result);
  url += "&reason=" + urlEncode(payload.reason);
  url += "&failed_count=" + String(payload.failedCount);
  url += "&lockout_until=" + String(payload.lockoutUntil);
  url += "&door_state=" + urlEncode(payload.doorState);
  return sendGetRequest(url);
}
