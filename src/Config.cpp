#include "Config.h"

namespace {
constexpr const char* DEFAULT_ADMIN_HASH =
    "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4";  // 1234
constexpr const char* DEFAULT_DEVICE_ID = "esp32-smart-server-01";
constexpr const char* DEFAULT_GSCRIPT_URL =
    "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec";

bool isStrictSchemaValid(const JsonDocument& doc) {
  return doc[ConfigKeys::WIFI_NETWORKS].is<JsonArray>() &&
         doc[ConfigKeys::USERS].is<JsonArray>() &&
         doc[ConfigKeys::SENSOR_INTERVAL].is<uint32_t>() &&
         doc[ConfigKeys::CLOUD_INTERVAL].is<uint32_t>() &&
         doc[ConfigKeys::WARN_THRESHOLD].is<float>() &&
         doc[ConfigKeys::STAGE2_THRESHOLD].is<float>() &&
         doc[ConfigKeys::FAN1_BASELINE].is<bool>() &&
         doc[ConfigKeys::MAX_FAILED].is<uint8_t>() &&
         doc[ConfigKeys::KEYPAD_LOCKOUT].is<uint32_t>() &&
         doc[ConfigKeys::SOLENOID_UNLOCK].is<uint32_t>() &&
         doc[ConfigKeys::GOOGLE_SCRIPT_URL].is<const char*>() &&
         doc[ConfigKeys::DEVICE_ID].is<const char*>();
}
}  // namespace

AppConfig::AppConfig() {
  for (auto& network : wifiNetworks) {
    network.ssid = "";
    network.password = "";
    network.enabled = false;
  }

  for (auto& user : users) {
    user.userId = "";
    user.displayName = "";
    user.pinHash = "";
    user.enabled = false;
  }

  users[0].userId = "admin";
  users[0].displayName = "Administrator";
  users[0].pinHash = DEFAULT_ADMIN_HASH;
  users[0].enabled = true;

  sensorReadIntervalSec = 5;
  cloudSendIntervalSec = 60;
  warnThresholdC = 27.0f;
  stage2ThresholdC = 28.0f;
  fan1BaselineOn = true;
  maxFailedAttempts = 3;
  keypadLockoutSec = 120;
  solenoidUnlockSec = 10;
  googleScriptUrl = DEFAULT_GSCRIPT_URL;
  deviceId = DEFAULT_DEVICE_ID;
}

ConfigManager::ConfigManager(const char* filename) : _filename(filename) {}

bool ConfigManager::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println(F("LittleFS mount failed"));
    return false;
  }
  return load();
}

bool ConfigManager::resetToDefaultsAndSave() {
  data = AppConfig();
  return save();
}

bool ConfigManager::load() {
  File file = LittleFS.open(_filename, "r");
  if (!file) {
    Serial.println(F("Config missing, creating defaults"));
    return resetToDefaultsAndSave();
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    Serial.printf("Config parse error: %s. Resetting defaults.\n", err.c_str());
    return resetToDefaultsAndSave();
  }

  if (!isStrictSchemaValid(doc)) {
    Serial.println(F("Config schema invalid/legacy. Resetting defaults."));
    return resetToDefaultsAndSave();
  }

  for (auto& network : data.wifiNetworks) {
    network = WiFiCredential{};
    network.enabled = false;
  }
  for (auto& user : data.users) {
    user = UserCredential{};
    user.enabled = false;
  }

  size_t i = 0;
  for (JsonObject net : doc[ConfigKeys::WIFI_NETWORKS].as<JsonArray>()) {
    if (i >= MAX_WIFI_NETWORKS) break;
    data.wifiNetworks[i].ssid = net[ConfigKeys::SSID].as<String>();
    data.wifiNetworks[i].password = net[ConfigKeys::PASSWORD].as<String>();
    data.wifiNetworks[i].enabled = net[ConfigKeys::ENABLED] | true;
    ++i;
  }

  i = 0;
  for (JsonObject user : doc[ConfigKeys::USERS].as<JsonArray>()) {
    if (i >= MAX_USERS) break;
    data.users[i].userId = user[ConfigKeys::USER_ID].as<String>();
    data.users[i].displayName = user[ConfigKeys::DISPLAY_NAME].as<String>();
    data.users[i].pinHash = user[ConfigKeys::PIN_HASH].as<String>();
    data.users[i].enabled = user[ConfigKeys::ENABLED] | true;
    ++i;
  }

  data.sensorReadIntervalSec = doc[ConfigKeys::SENSOR_INTERVAL].as<uint32_t>();
  data.cloudSendIntervalSec = doc[ConfigKeys::CLOUD_INTERVAL].as<uint32_t>();
  data.warnThresholdC = doc[ConfigKeys::WARN_THRESHOLD].as<float>();
  data.stage2ThresholdC = doc[ConfigKeys::STAGE2_THRESHOLD].as<float>();
  data.fan1BaselineOn = doc[ConfigKeys::FAN1_BASELINE].as<bool>();
  data.maxFailedAttempts = doc[ConfigKeys::MAX_FAILED].as<uint8_t>();
  data.keypadLockoutSec = doc[ConfigKeys::KEYPAD_LOCKOUT].as<uint32_t>();
  data.solenoidUnlockSec = doc[ConfigKeys::SOLENOID_UNLOCK].as<uint32_t>();
  data.googleScriptUrl = doc[ConfigKeys::GOOGLE_SCRIPT_URL].as<String>();
  data.deviceId = doc[ConfigKeys::DEVICE_ID].as<String>();

  if (data.googleScriptUrl.length() == 0 || data.deviceId.length() == 0 ||
      getUserCount() == 0) {
    Serial.println(F("Config incomplete. Resetting defaults."));
    return resetToDefaultsAndSave();
  }

  Serial.println(F("Config loaded"));
  return true;
}

bool ConfigManager::save() {
  JsonDocument doc;

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
  doc[ConfigKeys::FAN1_BASELINE] = data.fan1BaselineOn;
  doc[ConfigKeys::MAX_FAILED] = data.maxFailedAttempts;
  doc[ConfigKeys::KEYPAD_LOCKOUT] = data.keypadLockoutSec;
  doc[ConfigKeys::SOLENOID_UNLOCK] = data.solenoidUnlockSec;
  doc[ConfigKeys::GOOGLE_SCRIPT_URL] = data.googleScriptUrl;
  doc[ConfigKeys::DEVICE_ID] = data.deviceId;

  File file = LittleFS.open(_filename, "w");
  if (!file) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  serializeJson(doc, file);
  file.close();
  return true;
}

bool ConfigManager::addWiFi(const String& ssid, const String& password) {
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
