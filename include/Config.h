#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <array>

constexpr size_t MAX_WIFI_NETWORKS = 8;
constexpr size_t MAX_USERS = 10;

struct WiFiCredential {
  String ssid;
  String password;
  bool enabled = true;
};

struct UserCredential {
  String userId;
  String displayName;
  String pinHash;
  bool enabled = true;
};

struct AppConfig {
  std::array<WiFiCredential, MAX_WIFI_NETWORKS> wifiNetworks;
  std::array<UserCredential, MAX_USERS> users;
  uint32_t sensorReadIntervalSec = 5;
  uint32_t cloudSendIntervalSec = 60;
  float warnThresholdC = 27.0f;
  float stage2ThresholdC = 28.0f;
  bool fan1BaselineOn = true;
  uint8_t maxFailedAttempts = 3;
  uint32_t keypadLockoutSec = 120;
  uint32_t solenoidUnlockSec = 10;
  String googleScriptUrl;
  String deviceId;

  AppConfig();
};

namespace ConfigKeys {
constexpr const char* WIFI_NETWORKS = "wifi";
constexpr const char* USERS = "users";
constexpr const char* SSID = "s";
constexpr const char* PASSWORD = "p";
constexpr const char* ENABLED = "e";
constexpr const char* USER_ID = "id";
constexpr const char* DISPLAY_NAME = "n";
constexpr const char* PIN_HASH = "ph";
constexpr const char* SENSOR_INTERVAL = "sensor_interval";
constexpr const char* CLOUD_INTERVAL = "cloud_interval";
constexpr const char* WARN_THRESHOLD = "th_warn";
constexpr const char* STAGE2_THRESHOLD = "th_stage2";
constexpr const char* FAN1_BASELINE = "fan1_baseline";
constexpr const char* MAX_FAILED = "max_failed";
constexpr const char* KEYPAD_LOCKOUT = "lockout_secs";
constexpr const char* SOLENOID_UNLOCK = "unlock_secs";
constexpr const char* GOOGLE_SCRIPT_URL = "gscript_url";
constexpr const char* DEVICE_ID = "device_id";
}  // namespace ConfigKeys

class ConfigManager {
 public:
  explicit ConfigManager(const char* filename = "/config.json");

  [[nodiscard]] bool begin();
  [[nodiscard]] bool load();
  bool save();

  bool addWiFi(const String& ssid, const String& password);
  bool removeWiFi(const String& ssid);
  [[nodiscard]] size_t getWiFiCount() const;
  void clearAllWiFi();

  bool upsertUser(const UserCredential& user);
  bool removeUser(const String& userId);
  [[nodiscard]] size_t getUserCount() const;
  [[nodiscard]] const UserCredential* findUser(const String& userId) const;

  AppConfig data;

 private:
  const char* _filename;
  bool resetToDefaultsAndSave();
};
