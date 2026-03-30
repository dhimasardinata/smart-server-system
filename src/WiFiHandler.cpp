#include "WiFiHandler.h"

#include <HTTPClient.h>
#include <algorithm>

namespace {
const char* wifiStateName(WiFiManager::State state) {
  switch (state) {
    case WiFiManager::State::Idle:
      return "idle";
    case WiFiManager::State::Scanning:
      return "scanning";
    case WiFiManager::State::Connecting:
      return "connecting";
    case WiFiManager::State::Verifying:
      return "verifying";
    case WiFiManager::State::Connected:
      return "connected";
    case WiFiManager::State::ApMode:
      return "ap";
  }
  return "unknown";
}
}  // namespace

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

  if (_userScanPending && _state != State::Scanning) {
    processUserScanResults();
  }

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
      if (_keepApAlive)
        _dnsServer.processNextRequest();
      if (WiFi.status() == WL_CONNECTED) {
        onConnected();
      } else if (now - _lastAction > CONNECT_TIMEOUT) {
        Serial.println(F("WiFi: Connection timeout"));
        tryNextNetwork();
      }
      break;

    case State::Verifying: {
      if (_keepApAlive)
        _dnsServer.processNextRequest();
      onConnected();
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
  WiFi.mode(_keepApAlive ? WIFI_AP_STA : WIFI_STA);
  _state = State::Scanning;
  _lastAction = millis();
  WiFi.scanNetworks(true);
}

void WiFiManager::processScanResults() {
  int n = WiFi.scanComplete();
  if (n < 0)
    return;

  captureScanResults(true);

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

void WiFiManager::processUserScanResults() {
  const int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING)
    return;

  if (n < 0) {
    Serial.println(F("WiFi: Manual scan failed"));
    WiFi.scanDelete();
    _userScanPending = false;
  } else {
    captureScanResults(false);
    Serial.printf("WiFi: Manual scan cached %d networks\n",
                  static_cast<int>(_allScannedNetworks.size()));
  }

  if (_state == State::ApMode && !_keepApAlive) {
    WiFi.mode(WIFI_AP);
  }
}

void WiFiManager::captureScanResults(bool captureMatches) {
  const int n = WiFi.scanComplete();
  if (n < 0)
    return;

  _allScannedNetworks.clear();
  _matchedNetworks.clear();
  if (n > 0)
    _allScannedNetworks.reserve(static_cast<size_t>(n));

  for (int i = 0; i < n; ++i) {
    ScannedNetwork sn;
    sn.ssid = WiFi.SSID(i);
    sn.rssi = WiFi.RSSI(i);
    sn.open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
    sn.saved = false;

    for (const auto& saved : _config->data.wifiNetworks) {
      if (saved.enabled && saved.ssid == sn.ssid) {
        sn.saved = true;
        if (captureMatches) {
          _matchedNetworks.push_back({saved.ssid, saved.password, sn.rssi});
        }
        break;
      }
    }
    _allScannedNetworks.push_back(sn);
  }

  WiFi.scanDelete();
  _lastScanResultsMs = millis();
  _scanComplete = true;
  _userScanPending = false;
}

void WiFiManager::tryNextNetwork() {
  if (_currentNetIndex >= _matchedNetworks.size()) {
    onAllFailed();
    return;
  }

  const auto& net = _matchedNetworks[_currentNetIndex];
  Serial.printf("WiFi: Connecting to %s...\n", net.ssid.c_str());

  WiFi.mode(_keepApAlive ? WIFI_AP_STA : WIFI_STA);
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
  if (_keepApAlive) {
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _keepApAlive = false;
    Serial.println(F("WiFi: AP fallback stopped, local network access only"));
  }
  Serial.printf("WiFi: Connected to %s, IP: %s\n", WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str());
}

void WiFiManager::onAllFailed() {
  Serial.println(F("WiFi: All networks failed, starting AP mode"));
  startApMode();
}

void WiFiManager::startApMode() {
  Serial.println(F("WiFi: Preparing AP mode"));
  _keepApAlive = false;
  _dnsServer.stop();

  WiFi.mode(WIFI_MODE_NULL);
  delay(50);
  WiFi.setSleep(false);
  WiFi.mode(WIFI_AP);
  delay(50);

  bool apStarted = false;
  if (AP_PASS[0] == '\0') {
    Serial.println(F("WiFi: Starting open AP"));
    apStarted = WiFi.softAP(AP_SSID);
  } else {
    Serial.println(F("WiFi: Starting secured AP"));
    apStarted = WiFi.softAP(AP_SSID, AP_PASS);
  }

  if (!apStarted) {
    Serial.println(F("WiFi: softAP start failed"));
    _state = State::Idle;
    _lastAction = millis();
    return;
  }

  delay(150);
  const IPAddress apIp = WiFi.softAPIP();
  Serial.printf("WiFi: AP IP ready = %s\n", apIp.toString().c_str());

  _dnsServer.start(DNS_PORT, "*", apIp);
  _state = State::ApMode;
  _lastAction = millis();

  Serial.printf("WiFi: AP mode started - SSID: %s, IP: %s\n", AP_SSID,
                apIp.toString().c_str());
}

IPAddress WiFiManager::getIP() const {
  return (_state == State::ApMode) ? WiFi.softAPIP() : WiFi.localIP();
}

bool WiFiManager::isScanPending() const {
  return _state == State::Scanning || _userScanPending;
}

unsigned long WiFiManager::lastScanAgeMs() const {
  if (_lastScanResultsMs == 0)
    return 0;
  return millis() - _lastScanResultsMs;
}

bool WiFiManager::connectTo(const String& ssid, const String& password) {
  if (ssid.length() == 0)
    return false;

  _matchedNetworks.clear();
  _matchedNetworks.push_back({ssid, password, 0});
  _currentNetIndex = 0;
  _keepApAlive = (_state == State::ApMode);
  _lastAction = millis();

  Serial.printf("WiFi: Manual connect requested for %s\n", ssid.c_str());
  tryNextNetwork();
  return true;
}

const char* WiFiManager::stateName() const {
  return wifiStateName(_state);
}

void WiFiManager::requestScanRefresh(bool force) {
  if (_userScanPending || _state == State::Scanning)
    return;

  const bool cacheFresh = _scanComplete && lastScanAgeMs() < SCAN_CACHE_MS;
  if (!force && cacheFresh)
    return;

  const bool keepAp = (_state == State::ApMode) || _keepApAlive;
  WiFi.mode(keepAp ? WIFI_AP_STA : WIFI_STA);
  WiFi.scanDelete();
  WiFi.scanNetworks(true, true);
  _userScanPending = true;
}

std::vector<WiFiManager::ScannedNetwork> WiFiManager::scanNow() {
  requestScanRefresh(true);
  return _allScannedNetworks;
}

std::vector<WiFiManager::ScannedNetwork> WiFiManager::getScannedNetworks()
    const {
  return _allScannedNetworks;
}
