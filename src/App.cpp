#include "App.h"

#include "PinMap.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

namespace {
constexpr const char* MDNS_HOSTNAME = "monitor-server";
}

App::App() : _display(Pins::I2C_ADDR_LCD, Pins::LCD_COLS, Pins::LCD_ROWS) {}

void App::setRelay(uint8_t pin, bool on) {
  const uint8_t level =
      Pins::RELAY_ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW);
  digitalWrite(pin, level);
}

void App::setupRelays() {
  pinMode(Pins::RELAY_FAN1, OUTPUT);
  pinMode(Pins::RELAY_FAN2, OUTPUT);
  pinMode(Pins::RELAY_SOLENOID, OUTPUT);
  _fan1On = false;
  _fan2On = false;
  _solenoidOn = false;
  _solenoidUnlockUntilMs = 0;
  setRelay(Pins::RELAY_FAN1, false);
  setRelay(Pins::RELAY_FAN2, false);
  setRelay(Pins::RELAY_SOLENOID, false);
}

void App::setup() {
  Serial.begin(115200);
  delay(100);

  esp_task_wdt_deinit();
  esp_log_level_set("task_wdt", ESP_LOG_NONE);
  esp_log_level_set("esp32-hal-ledc", ESP_LOG_WARN);
  esp_log_level_set("Preferences", ESP_LOG_WARN);

  Wire.begin(Pins::SDA, Pins::SCL);

  if (!_config.begin()) Serial.println(F("Config init failed"));
  _sensors.setReadIntervalMs(_config.data.sensorReadIntervalSec * 1000UL);

  if (_display.begin()) _display.showStartup();

  _sensors.begin();
  setupRelays();
  _access.begin(&_config);

  _wifi.begin(&_config);
  for (int i = 0; i < 20; ++i) {
    delay(100);
    yield();
  }

  _network.begin(&_config, &_wifi, &_sensors, &_access);

  if (_wifi.isConnected()) setupOTA();

  _display.clear();
}

void App::setupOTA() {
  if (MDNS.begin(MDNS_HOSTNAME)) {
    Serial.printf("mDNS: %s.local\n", MDNS_HOSTNAME);
  }

  ArduinoOTA.setHostname(MDNS_HOSTNAME);
  ArduinoOTA.setPort(3232);
  ArduinoOTA.onStart([]() { Serial.println(F("OTA start")); });
  ArduinoOTA.onEnd([]() { Serial.println(F("OTA done")); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error[%u]\n", error);
  });
  ArduinoOTA.begin();
}

void App::updateThermalAndFans(const SensorData& data) {
  _warning = data.valid && (data.temperature > _config.data.warnThresholdC);
  _fan2On = data.valid && (data.temperature >= _config.data.stage2ThresholdC);
  _fan1On = _config.data.fan1BaselineOn || _warning || _fan2On;

  setRelay(Pins::RELAY_FAN1, _fan1On);
  setRelay(Pins::RELAY_FAN2, _fan2On);
}

void App::requestUnlock() {
  _solenoidOn = true;
  _solenoidUnlockUntilMs = millis() + (_config.data.solenoidUnlockSec * 1000UL);
  setRelay(Pins::RELAY_SOLENOID, true);
}

void App::updateSolenoid() {
  if (_solenoidOn && _solenoidUnlockUntilMs > 0 &&
      millis() >= _solenoidUnlockUntilMs) {
    _solenoidOn = false;
    _solenoidUnlockUntilMs = 0;
    setRelay(Pins::RELAY_SOLENOID, false);
  }
}

void App::updateDisplay(const SensorData& data) {
  _display.setWifiInfo(_wifi.isConnected(), _wifi.getIP().toString());
  _display.setTelemetry(data.temperature, data.humidity, data.valid, _fan1On,
                        _fan2On, _warning);
  _display.setSecurity(_solenoidOn ? "UNLOCKING" : "LOCKED", _access.lastMessage(),
                       _access.isLockoutActive(), _access.lockoutRemainingSec());
}

void App::resetToMonitoring() {
  _uiState = UIState::MONITORING;
  _pinBuf = "";
  _confirmBuf = "";
  _changePinStep = 0;
  _display.clear();
}

void App::buildUserSlotMap() {
  _userSlotCount = 0;
  for (size_t i = 0; i < MAX_USERS; ++i) {
    if (_config.data.users[i].userId.length() == 0 ||
        !_config.data.users[i].enabled)
      continue;
    if (_userListAction == 1 && i == 0) continue;
    if (_userSlotCount < MAX_SLOTS) {
      _userSlotMap[_userSlotCount++] = static_cast<uint8_t>(i);
    }
  }
}

void App::handleUIKey(char key) {
  if (key == NO_KEY) return;

  _uiIdleMs = millis();

  switch (_uiState) {
    case UIState::MONITORING: {
      if (key == 'A') {
        _uiState = UIState::PIN_ENTRY;
        _pinBuf = "";
        if (_access.isLockoutActive()) {
          _display.showPinEntry(0, true, _access.lockoutRemainingSec());
        } else {
          _display.showPinEntry(0);
        }
      }
      break;
    }

    case UIState::PIN_ENTRY: {
      if (_access.isLockoutActive()) {
        _display.showPinEntry(0, true, _access.lockoutRemainingSec());
        if (key == '*') resetToMonitoring();
        break;
      }
      if (key >= '0' && key <= '9' && _pinBuf.length() < 8) {
        _pinBuf += key;
        _display.showPinEntry(_pinBuf.length());
      } else if (key == '*') {
        resetToMonitoring();
      } else if (key == '#' && _pinBuf.length() >= 4) {
        AuthResult auth = _access.validatePin(_pinBuf);
        _pinBuf = "";
        if (auth.success) {
          _authUserId = auth.userId;
          _authDisplayName = auth.displayName;
          if (auth.isAdmin) {
            _uiState = UIState::ADMIN_MENU;
            _display.showAdminMenu();
          } else {
            _uiState = UIState::UNLOCK_OK;
            _unlockOkMs = millis();
            requestUnlock();
            _display.showUnlockOk(auth.displayName);
          }
        } else {
          if (_access.isLockoutActive()) {
            _display.showPinEntry(0, true, _access.lockoutRemainingSec());
          } else {
            _display.showMessage("PIN SALAH", "Coba lagi", false);
            delay(1500);
            _uiState = UIState::PIN_ENTRY;
            _display.showPinEntry(0);
          }
        }
      }
      break;
    }

    case UIState::UNLOCK_OK: {
      resetToMonitoring();
      break;
    }

    case UIState::ADMIN_MENU: {
      if (key == '*') {
        resetToMonitoring();
      } else if (key == '1') {
        requestUnlock();
        _uiState = UIState::UNLOCK_OK;
        _unlockOkMs = millis();
        _display.showUnlockOk(_authDisplayName);
      } else if (key == '2') {
        _userListAction = 0;
        buildUserSlotMap();
        _uiState = UIState::USER_LIST;
        _display.showUserList(_config.data.users, MAX_USERS, 0);
      } else if (key == '3') {
        _autoUserId = _access.generateUserId();
        _pinBuf = "";
        _uiState = UIState::ADD_USER;
        _display.showAddUser(_autoUserId, 0);
      } else if (key == '4') {
        _userListAction = 1;
        buildUserSlotMap();
        _uiState = UIState::USER_LIST;
        _display.showUserList(_config.data.users, MAX_USERS, 1);
      }
      break;
    }

    case UIState::USER_LIST: {
      if (key == '*') {
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '1' && key <= '9') {
        uint8_t slot = key - '1';
        if (slot < _userSlotCount) {
          uint8_t idx = _userSlotMap[slot];
          _selectedUserId = _config.data.users[idx].userId;
          if (_userListAction == 0) {
            _uiState = UIState::CHANGE_PIN;
            _changePinStep = 0;
            _pinBuf = "";
            _confirmBuf = "";
            _display.showChangePin(_selectedUserId, 0, 0);
          } else {
            _uiState = UIState::CONFIRM_DELETE;
            _display.showConfirmDelete(_selectedUserId);
          }
        }
      }
      break;
    }

    case UIState::CHANGE_PIN: {
      if (key == '*') {
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '0' && key <= '9') {
        if (_changePinStep == 0 && _pinBuf.length() < 8) {
          _pinBuf += key;
          _display.showChangePin(_selectedUserId, 0, _pinBuf.length());
        } else if (_changePinStep == 1 && _confirmBuf.length() < 8) {
          _confirmBuf += key;
          _display.showChangePin(_selectedUserId, 1, _confirmBuf.length());
        }
      } else if (key == '#') {
        if (_changePinStep == 0 && _pinBuf.length() >= 4) {
          _changePinStep = 1;
          _confirmBuf = "";
          _display.showChangePin(_selectedUserId, 1, 0);
        } else if (_changePinStep == 1 && _confirmBuf.length() >= 4) {
          if (_pinBuf == _confirmBuf) {
            String error;
            if (_access.changePin(_selectedUserId, _pinBuf, error)) {
              _display.showMessage("BERHASIL", "PIN diperbarui", true);
            } else {
              _display.showMessage("GAGAL", error.c_str(), false);
            }
          } else {
            _display.showMessage("GAGAL", "PIN tidak cocok", false);
          }
          delay(2000);
          _uiState = UIState::ADMIN_MENU;
          _display.showAdminMenu();
        }
      }
      break;
    }

    case UIState::ADD_USER: {
      if (key == '*') {
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '0' && key <= '9' && _pinBuf.length() < 8) {
        _pinBuf += key;
        _display.showAddUser(_autoUserId, _pinBuf.length());
      } else if (key == '#' && _pinBuf.length() >= 4) {
        String error;
        if (_access.upsertUser(_autoUserId, _autoUserId, _pinBuf, true, error)) {
          _display.showMessage("BERHASIL", "User ditambahkan", true);
        } else {
          _display.showMessage("GAGAL", error.c_str(), false);
        }
        delay(2000);
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      }
      break;
    }

    case UIState::CONFIRM_DELETE: {
      if (key == '*') {
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key == '1') {
        if (_config.removeUser(_selectedUserId)) {
          _display.showMessage("BERHASIL", "User dihapus", true);
        } else {
          _display.showMessage("GAGAL", "Gagal menghapus", false);
        }
        delay(2000);
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      }
      break;
    }
  }
}

void App::loop() {
  _wifi.update();
  _sensors.update();
  _access.update();

  if (_access.consumeUnlockRequest() && _uiState != UIState::UNLOCK_OK) {
    requestUnlock();
  }

  const SensorData data = _sensors.getData();
  updateThermalAndFans(data);
  updateSolenoid();

  if (_wifi.isConnected()) ArduinoOTA.handle();

  AccessEvent event;
  while (_access.popEvent(event)) {
    _network.logAccessEvent(event);
  }

  _network.update(data, _fan1On, _fan2On, _warning, _solenoidOn);
  updateDisplay(data);

  char key = _access.getKey();
  handleUIKey(key);

  if (_uiState == UIState::MONITORING) {
    static unsigned long lastDisplayRefresh = 0;
    if (millis() - lastDisplayRefresh > 1000) {
      lastDisplayRefresh = millis();
      _display.showMainScreen();
    }
  }

  if (_uiState == UIState::UNLOCK_OK &&
      millis() - _unlockOkMs > UNLOCK_DISPLAY_MS) {
    resetToMonitoring();
  }

  if (_uiState != UIState::MONITORING && _uiState != UIState::UNLOCK_OK &&
      millis() - _uiIdleMs > UI_TIMEOUT_MS) {
    resetToMonitoring();
  }

  delay(1);
}
