#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <array>

constexpr size_t MAX_WIFI_NETWORKS = 8;
constexpr size_t MAX_USERS = 10;

// Data satu jaringan yang disimpan di perangkat.
struct WiFiCredential {
  // Nama jaringan WiFi.
  String ssid;
  // Kata sandi jaringan.
  String password;
  // Penanda aktif atau tidak.
  bool enabled = true;
};

// Data satu pengguna yang boleh masuk.
struct UserCredential {
  // ID pengguna.
  String userId;
  // Nama yang ditampilkan di layar.
  String displayName;
  // Hash dari PIN, bukan PIN mentah.
  String pinHash;
  // Penanda aktif atau tidak.
  bool enabled = true;
};

// Semua pengaturan utama disatukan di sini.
struct AppConfig {
  // Daftar WiFi yang boleh dipakai.
  std::array<WiFiCredential, MAX_WIFI_NETWORKS> wifiNetworks;
  // Daftar pengguna yang diizinkan.
  std::array<UserCredential, MAX_USERS> users;
  // Jeda baca sensor dalam detik.
  uint32_t sensorReadIntervalSec = 5;
  // Jeda kirim data ke cloud dalam detik.
  uint32_t cloudSendIntervalSec = 60;
  // Ambang termal untuk peringatan.
  float warnThresholdC = 27.0f;
  // Ambang termal untuk alarm kedua.
  float stage2ThresholdC = 28.0f;
  // Ambang kelembapan untuk peringatan.
  float warnHumPct = 65.0f;
  // Ambang kelembapan untuk alarm kedua.
  float stage2HumPct = 75.0f;
  // Status dasar kipas pertama.
  bool fan1BaselineOn = true;
  // Batas salah PIN.
  uint8_t maxFailedAttempts = 3;
  // Lama penguncian sementara.
  uint32_t keypadLockoutSec = 120;
  // Lama solenoid dibuka.
  uint32_t solenoidUnlockSec = 5;
  // Alamat skrip Google Apps Script.
  String googleScriptUrl;
  // ID perangkat yang ikut dikirim ke cloud.
  String deviceId;

  // Isi default ditentukan di constructor.
  AppConfig();
};

namespace ConfigKeys {
// Nama kunci yang dipakai saat menyimpan pengaturan.
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
constexpr const char* WARN_HUM_THRESHOLD = "th_warn_h";
constexpr const char* STAGE2_HUM_THRESHOLD = "th_stage2_h";
constexpr const char* FAN1_BASELINE = "fan1_baseline";
constexpr const char* MAX_FAILED = "max_failed";
constexpr const char* KEYPAD_LOCKOUT = "lockout_secs";
constexpr const char* SOLENOID_UNLOCK = "unlock_secs";
constexpr const char* GOOGLE_SCRIPT_URL = "gscript_url";
constexpr const char* DEVICE_ID = "device_id";
}  // namespace ConfigKeys

class ConfigManager {
 public:
  // Pengurus baca/tulis setelan.
  explicit ConfigManager(const char* filename = "/config.json");

  // Membaca dan menulis pengaturan dari penyimpanan internal.
  [[nodiscard]] bool begin();
  [[nodiscard]] bool load();
  bool save();
  bool formatFileSystem();

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
