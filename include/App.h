#pragma once

#include "AccessController.h"
#include "Config.h"
#include "Display.h"
#include "NetworkServices.h"
#include "Sensors.h"
#include "WiFiHandler.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

enum class UIState : uint8_t {
  MONITORING,
  PIN_ENTRY,
  UNLOCK_OK,
  ADMIN_MENU,
  USER_LIST,
  CHANGE_PIN,
  ADD_USER,
  CONFIRM_DELETE,
  STATUS_MESSAGE
};

enum class AlertState : uint8_t {
  Idle,
  AccessGranted,
  AccessDenied,
  Lockout,
  ThermalWarning,
  ThermalCritical
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
  SemaphoreHandle_t _runtimeMutex = nullptr;
  TaskHandle_t _backgroundTask = nullptr;
  TaskHandle_t _otaTask = nullptr;
  bool _backgroundTaskEnabled = false;
  bool _otaTaskEnabled = false;

  bool _fan1On = false;
  bool _fan2On = false;
  bool _warning = false;
  bool _solenoidOn = false;
  bool _otaReady = false;
  unsigned long _solenoidUnlockUntilMs = 0;
  bool _alertOn = false;
  bool _fan1RelayApplied = false;
  bool _fan2RelayApplied = false;
  bool _solenoidRelayApplied = false;
  bool _alertRelayApplied = false;
  AlertState _alertState = AlertState::Idle;
  uint8_t _thermalTier = 0;
  unsigned long _alertUntilMs = 0;

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
  unsigned long _statusUntilMs = 0;
  unsigned long _lastApDisplayRefreshMs = 0;
  unsigned long _lastLockoutRefreshMs = 0;
  UIState _statusReturnState = UIState::MONITORING;

  static constexpr unsigned long UI_TIMEOUT_MS = 30000;
  static constexpr unsigned long UNLOCK_DISPLAY_MS = 3000;
  static constexpr unsigned long AP_DISPLAY_REFRESH_MS = 1000;
  static constexpr unsigned long LOCKOUT_REFRESH_MS = 1000;
  static constexpr uint8_t ALERT_QUEUE_SIZE = 6;

  AlertState _alertQueue[ALERT_QUEUE_SIZE] = {};
  uint8_t _alertQueueHead = 0;
  uint8_t _alertQueueTail = 0;
  uint8_t _alertQueueCount = 0;

  struct RuntimeSnapshot {
    SensorData data{};
    bool fan1On = false;
    bool fan2On = false;
    bool warning = false;
    bool solenoidOn = false;
    bool alertOn = false;
    AlertState alertState = AlertState::Idle;
  } _runtimeSnapshot;

  void setupOTA();
  void setupRelays();
  void startBackgroundTask();
  void startOtaTask();
  void publishRuntimeSnapshot(const SensorData& data);
  RuntimeSnapshot readRuntimeSnapshot();
  static void backgroundTaskEntry(void* context);
  static void otaTaskEntry(void* context);
  void backgroundTaskLoop();
  void otaTaskLoop();
  void updateThermalAndFans(const SensorData& data);
  void updateSolenoid();
  void updateAlert();
  void handleAccessAlert(const AccessEvent& event);
  void updateThermalAlertState(bool warning, bool critical);
  void enqueueAlert(AlertState state);
  void clearQueuedAlerts(bool stopCurrent = true);
  bool dequeueAlert(AlertState& state);
  void startAlert(AlertState state);
  void stopAlert();
  void setAlertRelay(bool on);
  [[nodiscard]] bool hasContinuousThermalAlert() const;
  [[nodiscard]] uint16_t alertDurationMs(AlertState state) const;
  [[nodiscard]] const char* alertStateName() const;
  static const char* alertStateName(AlertState state);
  void requestUnlock();
  void startUnlockSession(const String& displayName);
  void setRelay(uint8_t pin, bool on);
  void updateDisplay(const SensorData& data);
  void renderCurrentUiState();
  void showTransientMessage(const char* title, const char* msg, bool success,
                            UIState returnState, unsigned long durationMs);
  void handleUIKey(char key);
  void resetToMonitoring();
  void buildUserSlotMap();
  void popLastDigit(String& buffer);

  static constexpr uint8_t MAX_SLOTS = 10;
  uint8_t _userSlotCount = 0;
  uint8_t _userSlotMap[MAX_SLOTS] = {};
};
