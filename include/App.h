#pragma once

#include "AccessController.h"
#include "Config.h"
#include "Display.h"
#include "NetworkServices.h"
#include "Sensors.h"
#include "WiFiHandler.h"

enum class UIState : uint8_t {
  MONITORING,
  PIN_ENTRY,
  UNLOCK_OK,
  ADMIN_MENU,
  USER_LIST,
  CHANGE_PIN,
  ADD_USER,
  CONFIRM_DELETE
};

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

  UIState _uiState = UIState::MONITORING;
  String _pinBuf;
  String _confirmBuf;
  String _authUserId;
  String _authDisplayName;
  String _selectedUserId;
  String _autoUserId;
  uint8_t _changePinStep = 0;
  uint8_t _userListAction = 0;
  unsigned long _uiIdleMs = 0;
  unsigned long _unlockOkMs = 0;

  static constexpr unsigned long UI_TIMEOUT_MS = 30000;
  static constexpr unsigned long UNLOCK_DISPLAY_MS = 3000;

  void setupOTA();
  void setupRelays();
  void updateThermalAndFans(const SensorData& data);
  void updateSolenoid();
  void requestUnlock();
  void setRelay(uint8_t pin, bool on);
  void updateDisplay(const SensorData& data);
  void handleUIKey(char key);
  void resetToMonitoring();
  void buildUserSlotMap();

  static constexpr uint8_t MAX_SLOTS = 10;
  uint8_t _userSlotCount = 0;
  uint8_t _userSlotMap[MAX_SLOTS] = {};
};
