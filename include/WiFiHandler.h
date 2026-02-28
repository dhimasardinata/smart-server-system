#pragma once

#include "Config.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <vector>

class WiFiManager {
 public:
  enum class State : uint8_t {
    Idle,
    Scanning,
    Connecting,
    Verifying,
    Connected,
    ApMode
  };

  WiFiManager() = default;

  void begin(ConfigManager* config);
  void update();

  [[nodiscard]] State getState() const { return _state; }
  [[nodiscard]] bool isConnected() const { return _state == State::Connected; }
  [[nodiscard]] bool isApMode() const { return _state == State::ApMode; }
  [[nodiscard]] String getSSID() const { return WiFi.SSID(); }
  [[nodiscard]] int32_t getRSSI() const { return WiFi.RSSI(); }
  [[nodiscard]] IPAddress getIP() const;

  void startApMode();

  struct ScannedNetwork {
    String ssid;
    int32_t rssi;
    bool open;
    bool saved;
  };
  [[nodiscard]] std::vector<ScannedNetwork> getScannedNetworks() const;

 private:
  ConfigManager* _config = nullptr;
  State _state = State::Idle;

  unsigned long _lastAction = 0;
  size_t _currentNetIndex = 0;
  bool _scanComplete = false;

  struct MatchedNetwork {
    String ssid;
    String password;
    int32_t rssi;
  };
  std::vector<MatchedNetwork> _matchedNetworks;
  std::vector<ScannedNetwork> _allScannedNetworks;

  DNSServer _dnsServer;
  static constexpr const char* AP_SSID = "TempMonitor-Setup";
  static constexpr const char* AP_PASS = "";
  static constexpr uint8_t DNS_PORT = 53;

  static constexpr unsigned long SCAN_INTERVAL = 30000;
  static constexpr unsigned long CONNECT_TIMEOUT = 15000;
  static constexpr unsigned long VERIFY_TIMEOUT = 10000;
  static constexpr unsigned long RETRY_DELAY = 5000;

  static constexpr const char* CONNECTIVITY_CHECK_URL =
      "http://connectivitycheck.gstatic.com/generate_204";

  void startScan();
  void processScanResults();
  void tryNextNetwork();
  bool verifyInternet();
  void onConnected();
  void onAllFailed();
};
