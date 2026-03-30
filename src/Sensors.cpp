#include "Sensors.h"

#include <Wire.h>

namespace {
constexpr uint8_t SHT21_ADDRESS = 0x40;

bool probeI2cAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission(true) == 0;
}

void logI2cScan() {
  Serial.println(F("I2C scan result:"));

  uint8_t found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission(true) == 0) {
      Serial.printf(" - device at 0x%02X\n", address);
      ++found;
    }
    delay(1);
  }

  if (found == 0) {
    Serial.println(F(" - no I2C devices found"));
  }
}

const char* sensorTypeName(SHTSensor::SHTSensorType type) {
  switch (type) {
    case SHTSensor::SHT2X:
      return "SHT2x";
    case SHTSensor::SHT3X:
      return "SHT3x";
    default:
      return "unknown";
  }
}
}  // namespace

SHT21Sensor::SHT21Sensor() : _sht(SHTSensor::SHT2X) {}

bool SHT21Sensor::begin() {
  delay(20);

  if (!probeI2cAddress(SHT21_ADDRESS)) {
    Serial.println(F("SHT21 not detected at I2C address 0x40"));
    logI2cScan();
    return false;
  }

  if (!_sht.init(Wire)) {
    Serial.println(F("SHT21 init failed (arduino-sht)"));
    logI2cScan();
    return false;
  }

  _sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);

  Serial.print(F("SHT21 initialized via arduino-sht, type="));
  Serial.println(sensorTypeName(_sht.mSensorType));

  _ready = true;
  return true;
}

SensorData SHT21Sensor::read() {
  SensorData data{};

  if (!_ready) {
    data.valid = false;
    return data;
  }

  if (!_sht.readSample()) {
    data.valid = false;
    Serial.println(F("SHT21 read failed"));
    return data;
  }

  data.temperature = _sht.getTemperature();
  data.humidity = _sht.getHumidity();
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
