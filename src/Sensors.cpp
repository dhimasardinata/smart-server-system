#include "Sensors.h"

#include "I2CBus.h"

#include <Wire.h>

namespace {
constexpr uint8_t SHT21_ADDRESS = 0x40;
constexpr uint8_t SHT3X_ADDRESS = 0x44;
constexpr unsigned long SENSOR_RECONNECT_INTERVAL_MS = 1000;

void logI2cScan() {
  // Pengecekan ini membantu saat sensor tidak ketemu.
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
  // Nama sensor dipakai supaya hasilnya gampang dipahami.
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

bool SHT21Sensor::initialize(bool detailedLog, bool recoveredLog) {
  // Pastikan sensor benar-benar ada sebelum dipakai.
  if (!I2CBus::probe(SHT21_ADDRESS)) {
    if (detailedLog) {
      Serial.println(F("SHT21 not detected at I2C address 0x40"));
      logI2cScan();
    }
    _ready = false;
    return false;
  }

  if (!_sht.init(Wire)) {
    if (detailedLog) {
      Serial.println(F("SHT21 init failed (arduino-sht)"));
      logI2cScan();
    }
    _ready = false;
    return false;
  }

  _sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);
  _ready = true;

  if (recoveredLog) {
    Serial.print(F("SHT21 recovered, type="));
  } else {
    Serial.print(F("SHT21 initialized via arduino-sht, type="));
  }
  Serial.println(sensorTypeName(_sht.mSensorType));
  return true;
}

bool SHT21Sensor::begin() {
  delay(20);
  return initialize(true, false);
}

bool SHT21Sensor::ensureReady() {
  // Kalau sempat gagal, jangan dicoba terus-menerus.
  if (_ready) {
    return true;
  }

  const unsigned long now = millis();
  if (now - _lastReconnectAttemptMs < SENSOR_RECONNECT_INTERVAL_MS) {
    return false;
  }
  _lastReconnectAttemptMs = now;

  if (I2CBus::probe(SHT21_ADDRESS) && initialize(false, true)) {
    return true;
  }

  if (!I2CBus::recover("SHT21")) {
    Serial.println(F("I2C: SHT21 bus still held after recovery"));
  }
  return initialize(false, true);
}

SensorData SHT21Sensor::read() {
  SensorData data{};

  // Kalau belum siap, tandai hasil baca belum bisa dipakai.
  if (!ensureReady()) {
    data.valid = false;
    return data;
  }

  if (!_sht.readSample()) {
    data.valid = false;
    _ready = false;
    _lastReconnectAttemptMs = 0;
    Serial.println(F("SHT21 read failed, scheduling recovery"));
    if (!I2CBus::recover("SHT21 read")) {
      Serial.println(F("I2C: SHT21 bus still held after read failure"));
    }
    return data;
  }

  data.temperature = _sht.getTemperature();
  data.humidity = _sht.getHumidity();
  data.valid = true;

  return data;
}

SHT3xSensor::SHT3xSensor() : _sht(SHTSensor::SHT3X) {}

bool SHT3xSensor::initialize(bool detailedLog, bool recoveredLog) {
  // Sensor ini disiapkan dengan cara yang sama, cuma letaknya berbeda.
  if (!I2CBus::probe(SHT3X_ADDRESS)) {
    if (detailedLog) {
      Serial.println(F("SHT3x not detected at I2C address 0x44"));
      logI2cScan();
    }
    _ready = false;
    return false;
  }

  if (!_sht.init(Wire)) {
    if (detailedLog) {
      Serial.println(F("SHT3x init failed (arduino-sht)"));
      logI2cScan();
    }
    _ready = false;
    return false;
  }

  _sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);
  _ready = true;

  if (recoveredLog) {
    Serial.print(F("SHT3x recovered, type="));
  } else {
    Serial.print(F("SHT3x initialized via arduino-sht, type="));
  }
  Serial.println(sensorTypeName(_sht.mSensorType));
  return true;
}

bool SHT3xSensor::begin() {
  delay(20);
  return initialize(true, false);
}

bool SHT3xSensor::ensureReady() {
  if (_ready) {
    return true;
  }

  const unsigned long now = millis();
  if (now - _lastReconnectAttemptMs < SENSOR_RECONNECT_INTERVAL_MS) {
    return false;
  }
  _lastReconnectAttemptMs = now;

  if (I2CBus::probe(SHT3X_ADDRESS) && initialize(false, true)) {
    return true;
  }

  if (!I2CBus::recover("SHT3x")) {
    Serial.println(F("I2C: SHT3x bus still held after recovery"));
  }
  return initialize(false, true);
}

SensorData SHT3xSensor::read() {
  SensorData data{};

  if (!ensureReady()) {
    data.valid = false;
    return data;
  }

  if (!_sht.readSample()) {
    data.valid = false;
    _ready = false;
    _lastReconnectAttemptMs = 0;
    Serial.println(F("SHT3x read failed, scheduling recovery"));
    if (!I2CBus::recover("SHT3x read")) {
      Serial.println(F("I2C: SHT3x bus still held after read failure"));
    }
    return data;
  }

  data.temperature = _sht.getTemperature();
  data.humidity = _sht.getHumidity();
  data.valid = true;

  return data;
}

SensorManager::SensorManager() {}

void SensorManager::begin() {
  // Dua sensor disiapkan; kalau satu gagal, sistem tetap jalan.
  if (!_sht21.begin()) {
    Serial.println(F("SensorManager: SHT21 init failed"));
  }
  if (!_sht3x.begin()) {
    Serial.println(F("SensorManager: SHT3x init failed"));
  }
}

void SensorManager::update() {
  // Sensor tidak dibaca terlalu sering supaya kerja alat tetap enteng.
  if (millis() - _lastRead < _readIntervalMs) return;
  _lastRead = millis();

  SensorData tData = _sht21.read();
  SensorData hData = _sht3x.read();

  _data.valid = tData.valid && hData.valid;
  _data.temperature = tData.valid ? tData.temperature : 0.0f;
  _data.humidity = hData.valid ? hData.humidity : 0.0f;
}

SensorData SensorManager::getData() const { return _data; }
