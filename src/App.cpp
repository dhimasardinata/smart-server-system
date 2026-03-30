#include "App.h"

#include "PinMap.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

namespace {
constexpr const char* MDNS_HOSTNAME = "monitor-server";

uint32_t relayInactiveLevel(bool activeLow) { return activeLow ? 1U : 0U; }

uint32_t relayOutputLevel(bool activeLow, bool on) {
  return activeLow ? (on ? 0U : 1U) : (on ? 1U : 0U);
}

bool relayActiveLow(uint8_t pin) {
  if (pin == Pins::RELAY_SOLENOID) return Pins::SOLENOID_ACTIVE_LOW;
  if (pin == Pins::RELAY_ALERT) return Pins::ALERT_ACTIVE_LOW;
  return Pins::RELAY_ACTIVE_LOW;
}

void prepareRelayPin(uint8_t pin) {
  const gpio_num_t gpio = static_cast<gpio_num_t>(pin);
  const uint32_t inactiveLevel = relayInactiveLevel(relayActiveLow(pin));
  gpio_reset_pin(gpio);
  gpio_set_level(gpio, inactiveLevel);
  gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
}
}

App::App() : _display(Pins::I2C_ADDR_LCD, Pins::LCD_COLS, Pins::LCD_ROWS) {}

void App::setRelay(uint8_t pin, bool on) {
  const uint32_t level = relayOutputLevel(relayActiveLow(pin), on);
  gpio_set_level(static_cast<gpio_num_t>(pin), level);
}

void App::setupRelays() {
  prepareRelayPin(Pins::RELAY_FAN1);
  prepareRelayPin(Pins::RELAY_FAN2);
  prepareRelayPin(Pins::RELAY_SOLENOID);
  prepareRelayPin(Pins::RELAY_ALERT);
  _fan1On = false;
  _fan2On = false;
  _solenoidOn = false;
  _solenoidUnlockUntilMs = 0;
  _alertOn = false;
  _alertState = AlertState::Idle;
  _thermalTier = 0;
  _alertUntilMs = 0;
  _alertQueueHead = 0;
  _alertQueueTail = 0;
  _alertQueueCount = 0;
  setRelay(Pins::RELAY_FAN1, false);
  setRelay(Pins::RELAY_FAN2, false);
  setRelay(Pins::RELAY_SOLENOID, false);
  setRelay(Pins::RELAY_ALERT, false);
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
  if (_otaReady) return;

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
  _otaReady = true;
}

void App::updateThermalAndFans(const SensorData& data) {
  _warning = data.valid && (data.temperature > _config.data.warnThresholdC);
  _fan2On = data.valid && (data.temperature >= _config.data.stage2ThresholdC);
  _fan1On = _config.data.fan1BaselineOn || _warning || _fan2On;

  setRelay(Pins::RELAY_FAN1, _fan1On);
  setRelay(Pins::RELAY_FAN2, _fan2On);
  updateThermalAlertState(_warning, _fan2On);
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

void App::setAlertRelay(bool on) {
  _alertOn = on;
  setRelay(Pins::RELAY_ALERT, on);
}

bool App::hasContinuousThermalAlert() const { return _thermalTier > 0; }

uint16_t App::alertDurationMs(AlertState state) const {
  switch (state) {
    case AlertState::AccessDenied:
      return 2000;
    default:
      return 0;
  }
}

const char* App::alertStateName() const {
  switch (_alertState) {
    case AlertState::AccessGranted:
      return "ACCESS_GRANTED";
    case AlertState::AccessDenied:
      return "ACCESS_DENIED";
    case AlertState::Lockout:
      return "LOCKOUT";
    case AlertState::ThermalWarning:
      return "THERMAL_WARNING";
    case AlertState::ThermalCritical:
      return "THERMAL_CRITICAL";
    case AlertState::Idle:
    default:
      return "IDLE";
  }
}

void App::enqueueAlert(AlertState state) {
  if (state == AlertState::Idle || hasContinuousThermalAlert()) return;

  if (_alertQueueCount >= ALERT_QUEUE_SIZE) {
    _alertQueueHead = (_alertQueueHead + 1) % ALERT_QUEUE_SIZE;
    --_alertQueueCount;
  }

  _alertQueue[_alertQueueTail] = state;
  _alertQueueTail = (_alertQueueTail + 1) % ALERT_QUEUE_SIZE;
  ++_alertQueueCount;
}

void App::clearQueuedAlerts(bool stopCurrent) {
  _alertQueueHead = 0;
  _alertQueueTail = 0;
  _alertQueueCount = 0;

  if (stopCurrent) stopAlert();
}

bool App::dequeueAlert(AlertState& state) {
  if (_alertQueueCount == 0) return false;
  state = _alertQueue[_alertQueueHead];
  _alertQueueHead = (_alertQueueHead + 1) % ALERT_QUEUE_SIZE;
  --_alertQueueCount;
  return true;
}

void App::startAlert(AlertState state) {
  if (hasContinuousThermalAlert()) return;
  const uint16_t durationMs = alertDurationMs(state);
  if (durationMs == 0) return;

  _alertState = state;
  _alertUntilMs = millis() + durationMs;
  setAlertRelay(true);
}

void App::stopAlert() {
  _alertUntilMs = 0;
  _alertState = AlertState::Idle;
  setAlertRelay(false);
}

void App::updateThermalAlertState(bool warning, bool critical) {
  const uint8_t newTier = critical ? 2 : (warning ? 1 : 0);
  if (newTier == _thermalTier) return;

  _thermalTier = newTier;
  if (_thermalTier > 0) {
    _alertQueueHead = 0;
    _alertQueueTail = 0;
    _alertQueueCount = 0;
    _alertUntilMs = 0;
    _alertState = _thermalTier >= 2 ? AlertState::ThermalCritical
                                    : AlertState::ThermalWarning;
    setAlertRelay(true);
    return;
  }

  stopAlert();
}

void App::handleAccessAlert(const AccessEvent& event) {
  if (hasContinuousThermalAlert()) return;

  switch (event.type) {
    case AccessEventType::AccessGranted:
      clearQueuedAlerts(true);
      break;
    case AccessEventType::AccessDenied:
      clearQueuedAlerts(true);
      enqueueAlert(AlertState::AccessDenied);
      break;
    case AccessEventType::LockoutStarted:
      clearQueuedAlerts(true);
    case AccessEventType::LockoutEnded:
    default:
      break;
  }
}

void App::updateAlert() {
  if (hasContinuousThermalAlert()) {
    _alertState = _thermalTier >= 2 ? AlertState::ThermalCritical
                                    : AlertState::ThermalWarning;
    if (!_alertOn) setAlertRelay(true);
    return;
  }

  if (_alertState == AlertState::ThermalWarning ||
      _alertState == AlertState::ThermalCritical) {
    _alertState = AlertState::Idle;
  }

  if (_alertState == AlertState::Idle) {
    AlertState nextState;
    if (dequeueAlert(nextState)) {
      startAlert(nextState);
    } else if (_alertOn) {
      setAlertRelay(false);
    }
    return;
  }

  if (millis() >= _alertUntilMs) stopAlert();
}

void App::updateDisplay(const SensorData& data) {
  _display.setWifiInfo(_wifi.isConnected() || _wifi.isApMode(),
                       _wifi.getIP().toString());
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

void App::popLastDigit(String& buffer) {
  if (buffer.length() == 0) return;
  buffer.remove(buffer.length() - 1);
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
      } else if (key == 'D') {
        popLastDigit(_pinBuf);
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
        _display.showUserList(_config.data.users.data(), MAX_USERS, 0);
      } else if (key == '3') {
        _autoUserId = _access.generateUserId();
        _pinBuf = "";
        _uiState = UIState::ADD_USER;
        _display.showAddUser(_autoUserId, 0);
      } else if (key == '4') {
        _userListAction = 1;
        buildUserSlotMap();
        _uiState = UIState::USER_LIST;
        _display.showUserList(_config.data.users.data(), MAX_USERS, 1);
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
      } else if (key == 'D') {
        if (_changePinStep == 0) {
          popLastDigit(_pinBuf);
          _display.showChangePin(_selectedUserId, 0, _pinBuf.length());
        } else {
          popLastDigit(_confirmBuf);
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
      } else if (key == 'D') {
        popLastDigit(_pinBuf);
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

  if (_wifi.isConnected()) {
    setupOTA();
    ArduinoOTA.handle();
  } else if (_otaReady) {
    MDNS.end();
    _otaReady = false;
  }

  if (_access.consumeUnlockRequest() && _uiState != UIState::UNLOCK_OK) {
    requestUnlock();
  }

  const SensorData data = _sensors.getData();
  updateThermalAndFans(data);
  updateSolenoid();

  AccessEvent event;
  while (_access.popEvent(event)) {
    handleAccessAlert(event);
    _network.logAccessEvent(event);
  }

  updateAlert();
  _network.update(data, _fan1On, _fan2On, _warning, _solenoidOn, _alertOn,
                  alertStateName());
  updateDisplay(data);

  char key = _access.getKey();
  handleUIKey(key);

  if (_uiState == UIState::MONITORING) {
    static unsigned long lastDisplayRefresh = 0;
    if (millis() - lastDisplayRefresh > 1000) {
      lastDisplayRefresh = millis();
      if (_wifi.isApMode()) {
        const String apIp = _wifi.getIP().toString();
        _display.showApMode(std::string_view(apIp.c_str(), apIp.length()));
      } else {
        _display.showMainScreen();
      }
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
