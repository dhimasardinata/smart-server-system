#include "WiFiHandler.h"

#include <HTTPClient.h>
#include <algorithm>

void WiFiManager::begin(ConfigManager* config) {
  _config = config;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  if (_config->getWiFiCount() == 0) {
    Serial.println(F("No saved WiFi networks, starting AP mode"));
    startApMode();
  } else {
    startScan();
  }
}

void WiFiManager::update() {
  unsigned long now = millis();

  switch (_state) {
    case State::ApMode:
      _dnsServer.processNextRequest();
      break;

    case State::Scanning:
      if (WiFi.scanComplete() >= 0) {
        processScanResults();
      } else if (now - _lastAction > SCAN_INTERVAL) {
        startScan();
      }
      break;

    case State::Connecting:
      if (WiFi.status() == WL_CONNECTED) {
        _state = State::Verifying;
        _lastAction = now;
      } else if (now - _lastAction > CONNECT_TIMEOUT) {
        Serial.println(F("WiFi: Connection timeout"));
        tryNextNetwork();
      }
      break;

    case State::Verifying: {
      static unsigned long lastVerifyAttempt = 0;
      if (now - lastVerifyAttempt > 2000) {
        lastVerifyAttempt = now;
        if (verifyInternet()) {
          onConnected();
          break;
        }
      }
      
      if (now - _lastAction > VERIFY_TIMEOUT) {
        Serial.println(F("WiFi: Internet verification failed"));
        tryNextNetwork();
      }
      break;
    }

    case State::Connected:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("WiFi: Connection lost, rescanning"));
        _state = State::Idle;
        startScan();
      }
      break;

    case State::Idle:
      if (now - _lastAction > RETRY_DELAY) {
        startScan();
      }
      break;
  }
}

void WiFiManager::startScan() {
  Serial.println(F("WiFi: Starting scan..."));
  _state = State::Scanning;
  _lastAction = millis();
  WiFi.scanNetworks(true);
}

void WiFiManager::processScanResults() {
  int n = WiFi.scanComplete();
  if (n < 0) return;

  _allScannedNetworks.clear();
  _matchedNetworks.clear();

  for (int i = 0; i < n; ++i) {
    ScannedNetwork sn;
    sn.ssid = WiFi.SSID(i);
    sn.rssi = WiFi.RSSI(i);
    sn.open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
    sn.saved = false;

    for (const auto& saved : _config->data.wifiNetworks) {
      if (saved.enabled && saved.ssid == sn.ssid) {
        sn.saved = true;
        _matchedNetworks.push_back({saved.ssid, saved.password, sn.rssi});
        break;
      }
    }
    _allScannedNetworks.push_back(sn);
  }

  WiFi.scanDelete();

  std::sort(_matchedNetworks.begin(), _matchedNetworks.end(),
            [](const auto& a, const auto& b) { return a.rssi > b.rssi; });

  Serial.printf("WiFi: Found %d networks, %d matched\n", n,
                static_cast<int>(_matchedNetworks.size()));

  if (_matchedNetworks.empty()) {
    onAllFailed();
  } else {
    _currentNetIndex = 0;
    tryNextNetwork();
  }
}

void WiFiManager::tryNextNetwork() {
  if (_currentNetIndex >= _matchedNetworks.size()) {
    onAllFailed();
    return;
  }

  const auto& net = _matchedNetworks[_currentNetIndex];
  Serial.printf("WiFi: Connecting to %s...\n", net.ssid.c_str());

  WiFi.disconnect();
  delay(100);
  WiFi.begin(net.ssid.c_str(), net.password.c_str());

  _state = State::Connecting;
  _lastAction = millis();
  ++_currentNetIndex;
}

bool WiFiManager::verifyInternet() {
  HTTPClient http;
  http.setTimeout(5000);
  http.begin(CONNECTIVITY_CHECK_URL);
  int code = http.GET();
  http.end();
  return code == 204;
}

void WiFiManager::onConnected() {
  _state = State::Connected;
  Serial.printf("WiFi: Connected to %s, IP: %s\n", WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str());
}

void WiFiManager::onAllFailed() {
  Serial.println(F("WiFi: All networks failed, starting AP mode"));
  startApMode();
}

void WiFiManager::startApMode() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);

  _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  _state = State::ApMode;

  Serial.printf("WiFi: AP mode started - SSID: %s, IP: %s\n", AP_SSID,
                WiFi.softAPIP().toString().c_str());
}

IPAddress WiFiManager::getIP() const {
  return (_state == State::ApMode) ? WiFi.softAPIP() : WiFi.localIP();
}

std::vector<WiFiManager::ScannedNetwork> WiFiManager::getScannedNetworks()
    const {
  return _allScannedNetworks;
}
