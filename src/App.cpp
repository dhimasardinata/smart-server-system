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

void App::loop() {
  _wifi.update();
  _sensors.update();
  _access.update();

  if (_access.consumeUnlockRequest()) requestUnlock();

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
  _display.loop();

  static unsigned long lastDisplayRefresh = 0;
  if (millis() - lastDisplayRefresh > 1000) {
    lastDisplayRefresh = millis();
    _display.showMainScreen();
  }

  delay(1);
}
