#pragma once

#include "AccessController.h"
#include "Config.h"
#include "Display.h"
#include "NetworkServices.h"
#include "Sensors.h"
#include "WiFiHandler.h"

class App {
 public:
  App();

  void setup();
  void loop();

 private:
  ConfigManager _config;
  WiFiManager _wifi;
  SensorManager _sensors;
  AccessController _access;
  NetworkServices _network;
  Display _display;

  bool _fan1On = false;
  bool _fan2On = false;
  bool _warning = false;
  bool _solenoidOn = false;
  unsigned long _solenoidUnlockUntilMs = 0;

  void setupOTA();
  void setupRelays();
  void updateThermalAndFans(const SensorData& data);
  void updateSolenoid();
  void requestUnlock();
  void setRelay(uint8_t pin, bool on);
  void updateDisplay(const SensorData& data);
};
