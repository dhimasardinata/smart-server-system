#pragma once

#include "PinMap.h"

#include <Arduino.h>
#include <SHTSensor.h>

struct SensorData {
  float temperature;
  float humidity;
  bool valid;
};

class SHT21Sensor {
 public:
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

class SensorManager {
 public:
  SensorManager();

  void begin();
  void setReadIntervalMs(unsigned long intervalMs) { _readIntervalMs = intervalMs; }
  void update();
  [[nodiscard]] SensorData getData() const;

 private:
  SHT21Sensor _sht21;
  SensorData _data{};
  unsigned long _lastRead = 0;
  unsigned long _readIntervalMs = 5000;
};
