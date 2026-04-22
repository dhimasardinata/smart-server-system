#pragma once

#include "Config.h"

#include <Arduino.h>

#include <DNSServer.h>
#include <WiFi.h>
#include <vector>

class WiFiManager {
 public:
  enum class State : uint8_t {
    // Tahap sambungan jaringan dari cari sampai nyambung.
    Idle,
    Scanning,
    Connecting,
    Verifying,
    Connected,
    ApMode
  };

  WiFiManager() = default;

  // Mulai dari daftar jaringan yang sudah disimpan.
  void begin(ConfigManager* config);
  void update();

  [[nodiscard]] State getState() const { return _state; }
  [[nodiscard]] bool isConnected() const { return _state == State::Connected; }
  [[nodiscard]] bool isApMode() const { return _state == State::ApMode; }
  [[nodiscard]] String getSSID() const { return WiFi.SSID(); }
  [[nodiscard]] int32_t getRSSI() const { return WiFi.RSSI(); }
  [[nodiscard]] IPAddress getIP() const;
  [[nodiscard]] bool isScanPending() const;
  [[nodiscard]] unsigned long lastScanAgeMs() const;

  void startApMode();
  // Coba sambung ke jaringan yang dipilih.
  bool connectTo(const String& ssid, const String& password);
  void requestScanRefresh(bool force = false);
  [[nodiscard]] const char* stateName() const;

  struct ScannedNetwork {
    String ssid;
    int32_t rssi;
    bool open;
    bool saved;
  };
  std::vector<ScannedNetwork> scanNow();
  [[nodiscard]] std::vector<ScannedNetwork> getScannedNetworks() const;

 private:
  // Pengaturan utama dibaca dari sini.
  ConfigManager* _config = nullptr;
  // Status jaringan saat ini.
  State _state = State::Idle;

  unsigned long _lastAction = 0;
  size_t _currentNetIndex = 0;
  bool _scanComplete = false;
  bool _keepApAlive = false;
  bool _userScanPending = false;
  unsigned long _lastScanResultsMs = 0;

  struct MatchedNetwork {
    String ssid;
    String password;
    int32_t rssi;
  };
  // Daftar jaringan yang cocok saat pindai.
  std::vector<MatchedNetwork> _matchedNetworks;
  // Hasil pindai semua jaringan sekitar.
  std::vector<ScannedNetwork> _allScannedNetworks;

  // Nama jaringan sementara untuk penyetelan awal.
  DNSServer _dnsServer;
  static constexpr const char* AP_SSID = "TempMonitor-Setup";
  static constexpr const char* AP_PASS = "";
  static constexpr uint8_t DNS_PORT = 53;

  static constexpr unsigned long SCAN_INTERVAL = 30000;
  static constexpr unsigned long CONNECT_TIMEOUT = 15000;
  static constexpr unsigned long VERIFY_TIMEOUT = 10000;
  static constexpr unsigned long RETRY_DELAY = 5000;
  static constexpr unsigned long SCAN_CACHE_MS = 15000;

  static constexpr const char* CONNECTIVITY_CHECK_URL =
      "http://connectivitycheck.gstatic.com/generate_204";

  void startScan();
  void processScanResults();
  void processUserScanResults();
  void captureScanResults(bool captureMatches);
  void tryNextNetwork();
  bool verifyInternet();
  void onConnected();
  void onAllFailed();
};
