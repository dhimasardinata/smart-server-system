#include "Config.h"

namespace {
// PIN admin bawaan kalau file belum ada.
// Nilai ini sudah diubah dulu jadi bentuk aman.
constexpr const char* DEFAULT_ADMIN_HASH =
    "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4";  // 1234
// ID perangkat bawaan.
constexpr const char* DEFAULT_DEVICE_ID = "esp32-smart-server-01";
// Alamat bawaan untuk skrip Google Apps Script.
constexpr const char* DEFAULT_GSCRIPT_URL =
    "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec";

// Nama lama dari file lama agar konfigurasi lama tetap terbaca.
constexpr const char* LEGACY_WIFI_NETWORKS = "wifiNetworks";
constexpr const char* LEGACY_USERS = "userCredentials";
constexpr const char* LEGACY_SSID = "ssid";
constexpr const char* LEGACY_PASSWORD = "password";
constexpr const char* LEGACY_USER_ID = "userId";
constexpr const char* LEGACY_DISPLAY_NAME = "displayName";
constexpr const char* LEGACY_PIN_HASH = "pinHash";
constexpr const char* LEGACY_SENSOR_INTERVAL = "sensorReadIntervalSec";
constexpr const char* LEGACY_CLOUD_INTERVAL = "cloudSendIntervalSec";
constexpr const char* LEGACY_WARN_THRESHOLD = "warnThreshold";
constexpr const char* LEGACY_STAGE2_THRESHOLD = "stage2Threshold";
constexpr const char* LEGACY_FAN1_BASELINE = "fan1BaselineOn";
constexpr const char* LEGACY_MAX_FAILED = "maxFail";
constexpr const char* LEGACY_KEYPAD_LOCKOUT = "lockoutSecs";
constexpr const char* LEGACY_SOLENOID_UNLOCK = "unlockSecs";
constexpr const char* LEGACY_GOOGLE_SCRIPT_URL = "googleScriptUrl";
constexpr const char* LEGACY_DEVICE_ID = "deviceId";

template <typename TContainer>
JsonVariantConst getField(const TContainer& container, const char* primary,
                          const char* legacy, bool* usedLegacy = nullptr) {
  // Baca nama field baru dulu; kalau belum ada, coba nama lama agar config
  // lama tetap bisa dipakai.
  // Dengan cara ini, file lama masih bisa dibaca tanpa rusak.
  JsonVariantConst value = container[primary];
  if (!value.isNull()) return value;
  if (legacy == nullptr) return JsonVariantConst();

  value = container[legacy];
  if (!value.isNull() && usedLegacy != nullptr) *usedLegacy = true;
  return value;
}

template <typename TValue, typename TContainer>
bool readField(const TContainer& container, const char* primary,
               const char* legacy, TValue& out, bool* usedLegacy = nullptr) {
  // Ambil satu nilai dari JSON ke variabel biasa.
  JsonVariantConst value = getField(container, primary, legacy, usedLegacy);
  if (value.isNull()) return false;
  out = value.as<TValue>();
  return true;
}

template <typename TContainer>
bool readBoolField(const TContainer& container, const char* primary,
                   const char* legacy, bool& out,
                   bool* usedLegacy = nullptr) {
  // Nilai benar/salah kadang disimpan sebagai teks atau angka.
  JsonVariantConst value = getField(container, primary, legacy, usedLegacy);
  if (value.isNull()) return false;

  if (value.is<bool>()) {
    out = value.as<bool>();
    return true;
  }
  if (value.is<const char*>()) {
    String text = value.as<String>();
    text.trim();
    text.toLowerCase();
    if (text == "true" || text == "1") {
      out = true;
      return true;
    }
    if (text == "false" || text == "0") {
      out = false;
      return true;
    }
  }
  if (value.is<int>() || value.is<long>() || value.is<unsigned int>() ||
      value.is<uint32_t>()) {
    out = value.as<long>() != 0;
    return true;
  }

  return false;
}

template <typename TContainer>
JsonArrayConst readArrayField(const TContainer& container, const char* primary,
                              const char* legacy,
                              bool* usedLegacy = nullptr) {
  // Ambil data yang bentuknya daftar.
  JsonVariantConst value = getField(container, primary, legacy, usedLegacy);
  if (value.is<JsonArrayConst>()) return value.as<JsonArrayConst>();
  return JsonArrayConst();
}
}  // namespace

AppConfig::AppConfig() {
  // Nilai default ini dipakai saat konfigurasi belum ada atau rusak.
  // Artinya, alat tetap punya patokan awal yang aman.
  for (auto& network : wifiNetworks) {
    // Kosongkan semua slot WiFi dulu.
    network.ssid = "";
    network.password = "";
    network.enabled = false;
  }

  for (auto& user : users) {
    // Kosongkan semua slot pengguna dulu.
    user.userId = "";
    user.displayName = "";
    user.pinHash = "";
    user.enabled = false;
  }

  // Siapkan admin bawaan.
  users[0].userId = "admin";
  users[0].displayName = "Administrator";
  users[0].pinHash = DEFAULT_ADMIN_HASH;
  users[0].enabled = true;

  // Setelan sensor dan cloud bawaan.
  sensorReadIntervalSec = 5;
  cloudSendIntervalSec = 60;
  // Batas termal bawaan.
  warnThresholdC = 27.0f;
  stage2ThresholdC = 28.0f;
  warnHumPct = 65.0f;
  stage2HumPct = 75.0f;
  // Kipas pertama aktif dari dasar.
  fan1BaselineOn = true;
  // Batas salah PIN dan waktu kunci.
  maxFailedAttempts = 3;
  keypadLockoutSec = 120;
  // Lama buka solenoid.
  solenoidUnlockSec = 5;
  // Alamat layanan cloud dan ID perangkat.
  googleScriptUrl = DEFAULT_GSCRIPT_URL;
  deviceId = DEFAULT_DEVICE_ID;
}

ConfigManager::ConfigManager(const char* filename) : _filename(filename) {}

bool ConfigManager::begin() {
  // LittleFS adalah "memori kecil" di ESP32 untuk menyimpan file konfigurasi.
  // File di sini dipakai supaya setelan tidak hilang saat alat dimatikan.
  if (!LittleFS.begin(false)) {
    // Kalau gagal dibuka, coba format dulu.
    Serial.println(F("LittleFS mount failed, formatting"));
    if (!LittleFS.format()) {
      // Kalau format juga gagal, berhenti.
      Serial.println(F("LittleFS format failed"));
      return false;
    }
    if (!LittleFS.begin(false)) {
      // Kalau masih gagal, berarti memori memang bermasalah.
      Serial.println(F("LittleFS mount failed after format"));
      return false;
    }
  }
  // Kalau berhasil, lanjut baca file konfigurasi.
  Serial.println(F("LittleFS mounted"));
  return load();
}

bool ConfigManager::resetToDefaultsAndSave() {
  // Kembalikan semua pengaturan ke nilai bawaan lalu simpan.
  // Ini dipakai kalau file lama rusak atau belum ada.
  data = AppConfig();
  return save();
}

bool ConfigManager::load() {
  // Buka file konfigurasi dari memori internal.
  // Kalau file belum ada, default langsung dibuat.
  File file = LittleFS.open(_filename, "r");
  if (!file) {
    // Kalau file belum ada, buat dari default.
    Serial.println(F("Config missing, creating defaults"));
    return resetToDefaultsAndSave();
  }

  // Baca isi file ke dokumen JSON.
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    // Kalau isi file rusak, balikan ke default.
    // Lebih baik reset daripada meneruskan data yang salah.
    Serial.printf("Config parse error: %s. Resetting defaults.\n", err.c_str());
    return resetToDefaultsAndSave();
  }

  // Mulai dari nilai bawaan dulu.
  // Lalu timpa dengan isi file kalau ada.
  data = AppConfig();

  bool recognized = false;
  bool migrated = false;

  // WiFi disimpan sebagai daftar, jadi kita baca satu per satu.
  // Setiap slot dibaca hanya kalau memang ada isinya.
  size_t i = 0;
  for (JsonObjectConst net :
       readArrayField(doc, ConfigKeys::WIFI_NETWORKS, LEGACY_WIFI_NETWORKS,
                      &migrated)) {
    if (i >= MAX_WIFI_NETWORKS) break;

    WiFiCredential network;
    network.enabled = true;
    readField(net, ConfigKeys::SSID, LEGACY_SSID, network.ssid, &migrated);
    readField(net, ConfigKeys::PASSWORD, LEGACY_PASSWORD, network.password,
              &migrated);
    readBoolField(net, ConfigKeys::ENABLED, "enabled", network.enabled,
                  &migrated);
    if (network.ssid.length() == 0) continue;

    data.wifiNetworks[i++] = network;
    recognized = true;
  }

  // User juga disimpan sebagai daftar dengan jumlah terbatas.
  // Slot kosong diabaikan supaya daftar tetap rapi.
  i = 0;
  for (JsonObjectConst user :
       readArrayField(doc, ConfigKeys::USERS, LEGACY_USERS, &migrated)) {
    if (i >= MAX_USERS) break;

    UserCredential credential;
    credential.enabled = true;
    readField(user, ConfigKeys::USER_ID, LEGACY_USER_ID, credential.userId,
              &migrated);
    readField(user, ConfigKeys::DISPLAY_NAME, LEGACY_DISPLAY_NAME,
              credential.displayName, &migrated);
    readField(user, ConfigKeys::PIN_HASH, LEGACY_PIN_HASH, credential.pinHash,
              &migrated);
    readBoolField(user, ConfigKeys::ENABLED, "enabled", credential.enabled,
                  &migrated);
    if (credential.userId.length() == 0) continue;

    data.users[i++] = credential;
    recognized = true;
  }

  recognized |= readField(doc, ConfigKeys::SENSOR_INTERVAL,
                          LEGACY_SENSOR_INTERVAL, data.sensorReadIntervalSec,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::CLOUD_INTERVAL,
                          LEGACY_CLOUD_INTERVAL, data.cloudSendIntervalSec,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::WARN_THRESHOLD,
                          LEGACY_WARN_THRESHOLD, data.warnThresholdC,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::STAGE2_THRESHOLD,
                          LEGACY_STAGE2_THRESHOLD, data.stage2ThresholdC,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::WARN_HUM_THRESHOLD,
                          nullptr, data.warnHumPct,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::STAGE2_HUM_THRESHOLD,
                          nullptr, data.stage2HumPct,
                          &migrated);
  recognized |= readBoolField(doc, ConfigKeys::FAN1_BASELINE,
                              LEGACY_FAN1_BASELINE, data.fan1BaselineOn,
                              &migrated);
  recognized |= readField(doc, ConfigKeys::MAX_FAILED, LEGACY_MAX_FAILED,
                          data.maxFailedAttempts, &migrated);
  recognized |= readField(doc, ConfigKeys::KEYPAD_LOCKOUT,
                          LEGACY_KEYPAD_LOCKOUT, data.keypadLockoutSec,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::SOLENOID_UNLOCK,
                          LEGACY_SOLENOID_UNLOCK, data.solenoidUnlockSec,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::GOOGLE_SCRIPT_URL,
                          LEGACY_GOOGLE_SCRIPT_URL, data.googleScriptUrl,
                          &migrated);
  recognized |= readField(doc, ConfigKeys::DEVICE_ID, LEGACY_DEVICE_ID,
                          data.deviceId, &migrated);

  if (!recognized) {
    // Kalau strukturnya tidak dikenali, jangan pakai isi lama.
    Serial.println(F("Config schema unrecognized. Resetting defaults."));
    return resetToDefaultsAndSave();
  }

  bool repaired = false;
  // Pastikan field penting selalu ada walaupun file lama kurang lengkap.
  if (data.googleScriptUrl.length() == 0) {
    // Kalau alamat cloud kosong, isi dengan bawaan.
    data.googleScriptUrl = DEFAULT_GSCRIPT_URL;
    repaired = true;
  }
  if (data.deviceId.length() == 0) {
    data.deviceId = DEFAULT_DEVICE_ID;
    repaired = true;
  }
  if (getUserCount() == 0) {
    data.users[0].userId = "admin";
    data.users[0].displayName = "Administrator";
    data.users[0].pinHash = DEFAULT_ADMIN_HASH;
    data.users[0].enabled = true;
    repaired = true;
    Serial.println(F("Config has no users. Restored default admin."));
  }

  if (migrated) {
    Serial.println(F("Config legacy/missing fields migrated to current schema."));
  }

  if (migrated || repaired) {
    save();
  }

  Serial.println(F("Config loaded"));
  return true;
}

bool ConfigManager::save() {
  static constexpr const char* TEMP_FILENAME = "/config.tmp";
  // Buat dokumen JSON baru dari data saat ini.
  JsonDocument doc;

  // Simpan ke file sementara dulu supaya kalau listrik mati di tengah jalan,
  // file config lama tidak langsung rusak.
  JsonArray wifiArr = doc[ConfigKeys::WIFI_NETWORKS].to<JsonArray>();
  for (const auto& network : data.wifiNetworks) {
    if (network.ssid.length() == 0) continue;
    JsonObject net = wifiArr.add<JsonObject>();
    net[ConfigKeys::SSID] = network.ssid;
    net[ConfigKeys::PASSWORD] = network.password;
    net[ConfigKeys::ENABLED] = network.enabled;
  }

  JsonArray usersArr = doc[ConfigKeys::USERS].to<JsonArray>();
  for (const auto& user : data.users) {
    if (user.userId.length() == 0) continue;
    JsonObject item = usersArr.add<JsonObject>();
    item[ConfigKeys::USER_ID] = user.userId;
    item[ConfigKeys::DISPLAY_NAME] = user.displayName;
    item[ConfigKeys::PIN_HASH] = user.pinHash;
    item[ConfigKeys::ENABLED] = user.enabled;
  }

  doc[ConfigKeys::SENSOR_INTERVAL] = data.sensorReadIntervalSec;
  doc[ConfigKeys::CLOUD_INTERVAL] = data.cloudSendIntervalSec;
  doc[ConfigKeys::WARN_THRESHOLD] = data.warnThresholdC;
  doc[ConfigKeys::STAGE2_THRESHOLD] = data.stage2ThresholdC;
  doc[ConfigKeys::WARN_HUM_THRESHOLD] = data.warnHumPct;
  doc[ConfigKeys::STAGE2_HUM_THRESHOLD] = data.stage2HumPct;
  doc[ConfigKeys::FAN1_BASELINE] = data.fan1BaselineOn;
  doc[ConfigKeys::MAX_FAILED] = data.maxFailedAttempts;
  doc[ConfigKeys::KEYPAD_LOCKOUT] = data.keypadLockoutSec;
  doc[ConfigKeys::SOLENOID_UNLOCK] = data.solenoidUnlockSec;
  doc[ConfigKeys::GOOGLE_SCRIPT_URL] = data.googleScriptUrl;
  doc[ConfigKeys::DEVICE_ID] = data.deviceId;

  File file = LittleFS.open(TEMP_FILENAME, "w");
  if (!file) {
    // Kalau file tidak bisa dibuka untuk menulis, gagal.
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  // Tulis dokumen JSON ke file.
  const size_t written = serializeJson(doc, file);
  file.flush();
  file.close();

  if (written == 0) {
    Serial.println(F("Failed to serialize config"));
    LittleFS.remove(TEMP_FILENAME);
    return false;
  }

  // Ganti file lama dengan file baru hanya setelah penulisan sukses.
  LittleFS.remove(_filename);
  if (!LittleFS.rename(TEMP_FILENAME, _filename)) {
    Serial.println(F("Failed to replace config file"));
    LittleFS.remove(TEMP_FILENAME);
    return false;
  }

  Serial.printf("Config saved: wifi=%u users=%u\n",
                static_cast<unsigned>(getWiFiCount()),
                static_cast<unsigned>(getUserCount()));
  return true;
}

bool ConfigManager::formatFileSystem() {
  // Dipakai saat storage rusak dan perlu dibersihkan total.
  Serial.println(F("Formatting LittleFS and restoring defaults"));
  LittleFS.end();

  if (!LittleFS.format()) {
    Serial.println(F("LittleFS format failed"));
    return false;
  }
  if (!LittleFS.begin(false)) {
    Serial.println(F("LittleFS remount failed after format"));
    return false;
  }

  data = AppConfig();
  return save();
}

bool ConfigManager::addWiFi(const String& ssid, const String& password) {
  // Kalau SSID sudah ada, kita cukup update password-nya.
  for (auto& network : data.wifiNetworks) {
    if (network.ssid == ssid) {
      network.password = password;
      network.enabled = true;
      return save();
    }
  }

  for (auto& network : data.wifiNetworks) {
    if (network.ssid.length() == 0) {
      network.ssid = ssid;
      network.password = password;
      network.enabled = true;
      return save();
    }
  }
  return false;
}

bool ConfigManager::removeWiFi(const String& ssid) {
  for (auto& network : data.wifiNetworks) {
    if (network.ssid == ssid) {
      network.ssid = "";
      network.password = "";
      network.enabled = false;
      return save();
    }
  }
  return false;
}

size_t ConfigManager::getWiFiCount() const {
  size_t count = 0;
  for (const auto& network : data.wifiNetworks) {
    if (network.ssid.length() > 0 && network.enabled) ++count;
  }
  return count;
}

void ConfigManager::clearAllWiFi() {
  for (auto& network : data.wifiNetworks) {
    network.ssid = "";
    network.password = "";
    network.enabled = false;
  }
  save();
}

bool ConfigManager::upsertUser(const UserCredential& user) {
  if (user.userId.length() == 0 || user.pinHash.length() == 0) return false;

  // "Upsert" artinya update kalau ada, tambah kalau belum ada.
  for (auto& existing : data.users) {
    if (existing.userId == user.userId) {
      existing.displayName = user.displayName;
      existing.pinHash = user.pinHash;
      existing.enabled = user.enabled;
      return save();
    }
  }

  for (auto& existing : data.users) {
    if (existing.userId.length() == 0) {
      existing = user;
      return save();
    }
  }
  return false;
}

bool ConfigManager::removeUser(const String& userId) {
  for (auto& user : data.users) {
    if (user.userId == userId) {
      // Jangan hapus user terakhir yang masih aktif, supaya sistem tidak terkunci.
      if (user.enabled && getUserCount() <= 1) {
        Serial.println(F("Refusing to delete last enabled user"));
        return false;
      }
      user.userId = "";
      user.displayName = "";
      user.pinHash = "";
      user.enabled = false;
      return save();
    }
  }
  return false;
}

size_t ConfigManager::getUserCount() const {
  size_t count = 0;
  for (const auto& user : data.users) {
    if (user.userId.length() > 0 && user.enabled) ++count;
  }
  return count;
}

const UserCredential* ConfigManager::findUser(const String& userId) const {
  for (const auto& user : data.users) {
    if (user.userId == userId && user.enabled) return &user;
  }
  return nullptr;
}
