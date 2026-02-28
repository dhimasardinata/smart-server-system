#pragma once

#include "PinMap.h"

#include <Arduino.h>
#include <HTU2xD_SHT2x_Si70xx.h>

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
  HTU2xD_SHT2x_SI70xx _sht;
  bool _ready = false;
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
