#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <string_view>

struct UserCredential;

class Display {
 public:
  Display(uint8_t addr, uint8_t cols, uint8_t rows);

  bool begin();
  void clear();
  void loop();

  void print(uint8_t col, uint8_t row, std::string_view text);
  void printCenter(uint8_t row, std::string_view text);
  void printRow(uint8_t row, const char* text);

  void showStartup();
  void showApMode(std::string_view ip);
  void showError(std::string_view message);
  void showMainScreen();

  void showPinEntry(uint8_t pinLen, bool lockout = false, uint32_t lockSec = 0);
  void showUnlockOk(const String& name);
  void showAdminMenu();
  void showUserList(const UserCredential* users, size_t maxUsers, uint8_t action);
  void showChangePin(const String& userId, uint8_t step, uint8_t len);
  void showAddUser(const String& autoId, uint8_t pinLen);
  void showConfirmDelete(const String& userId);
  void showMessage(const char* title, const char* msg, bool success);

  void setWifiInfo(bool connected, const String& ip);
  void setTelemetry(float temperature, float humidity, bool valid, bool fan1On,
                    bool fan2On, bool warning);
  void setSecurity(const String& doorState, const String& accessMessage,
                   bool lockoutActive, uint32_t lockoutRemainSec);

  [[nodiscard]] bool isReady() const { return _ready; }

 private:
  LiquidCrystal_I2C _lcd;
  uint8_t _addr;
  uint8_t _cols;
  uint8_t _rows;
  bool _ready = false;

  bool _wifiConnected = false;
  String _ipAddress = "-";
  float _temperature = 0.0f;
  float _humidity = 0.0f;
  bool _sensorValid = false;
  bool _fan1On = false;
  bool _fan2On = false;
  bool _warning = false;
  String _doorState = "LOCKED";
  String _accessMessage = "READY";
  bool _lockoutActive = false;
  uint32_t _lockoutRemainSec = 0;

  unsigned long _lastScrollTime = 0;
  int _scrollOffset = 0;
  static constexpr unsigned long SCROLL_INTERVAL = 350;

  void renderHeaderScroll();
  void clearRow(uint8_t row);
};
