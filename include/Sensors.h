#pragma once

#include "PinMap.h"

#include <Arduino.h>
#include <SHTSensor.h>

// Satu sampel pembacaan suhu dan kelembapan.
struct SensorData {
  // Hasil suhu.
  float temperature;
  // Hasil kelembapan.
  float humidity;
  // Penanda hasil baca sah.
  bool valid;
};

class SHT21Sensor {
 public:
  // Pengurus sensor SHT21.
  SHT21Sensor();

  [[nodiscard]] bool begin();
  [[nodiscard]] SensorData read();
  [[nodiscard]] bool isReady() const { return _ready; }

 private:
  SHTSensor _sht;
  bool _ready = false;
  unsigned long _lastReconnectAttemptMs = 0;

  [[nodiscard]] bool ensureReady();
  [[nodiscard]] bool initialize(bool detailedLog, bool recoveredLog);
};

class SHT3xSensor {
 public:
  // Pengurus sensor SHT3x.
  SHT3xSensor();

  [[nodiscard]] bool begin();
  [[nodiscard]] SensorData read();
  [[nodiscard]] bool isReady() const { return _ready; }

 private:
  SHTSensor _sht;
  bool _ready = false;
  unsigned long _lastReconnectAttemptMs = 0;

  [[nodiscard]] bool ensureReady();
  [[nodiscard]] bool initialize(bool detailedLog, bool recoveredLog);
};

class SensorManager {
 public:
  // Pengurus gabungan dua sensor.
  SensorManager();

  void begin();
  // Interval baca bisa diubah dari menu konfigurasi.
  void setReadIntervalMs(unsigned long intervalMs) { _readIntervalMs = intervalMs; }
  void update();
  [[nodiscard]] SensorData getData() const;

 private:
  SHT21Sensor _sht21;
  SHT3xSensor _sht3x;
  SensorData _data{};
  unsigned long _lastRead = 0;
  unsigned long _readIntervalMs = 5000;
};
