#include "Sensors.h"
#include <Wire.h>

SHT21Sensor::SHT21Sensor() : _sht(SHT2x_SENSOR, HUMD_12BIT_TEMP_14BIT) {}

bool SHT21Sensor::begin() {
  if (!_sht.begin(SDA, SCL)) {
    Serial.println(F("SHT21 not responding"));
    return false;
  }

  Serial.println(F("SHT21 initialized (HTU2xD_SHT2x_Si70xx library)"));
  Serial.print(F("Firmware version: 0x"));
  Serial.println(_sht.readFirmwareVersion(), HEX);

  _ready = true;
  return true;
}

SensorData SHT21Sensor::read() {
  SensorData data{};

  if (!_ready) {
    data.valid = false;
    return data;
  }

  float temperature = _sht.readTemperature();
  float humidity = _sht.readHumidity();

  if (temperature == HTU2XD_SHT2X_SI70XX_ERROR || humidity == HTU2XD_SHT2X_SI70XX_ERROR) {
    data.valid = false;
    Serial.println(F("SHT21 read failed (CRC error)"));
    return data;
  }

  data.temperature = temperature;
  data.humidity = _sht.getCompensatedHumidity(temperature);
  data.valid = true;

  return data;
}

SensorManager::SensorManager() {}

void SensorManager::begin() {
  if (!_sht21.begin()) {
    Serial.println(F("SensorManager: SHT21 init failed"));
  }
}

void SensorManager::update() {
  if (millis() - _lastRead < _readIntervalMs) return;
  _lastRead = millis();

  _data = _sht21.read();
}

SensorData SensorManager::getData() const { return _data; }
