#include "Display.h"

#include <time.h>

Display::Display(uint8_t addr, uint8_t cols, uint8_t rows)
    : _lcd(addr, cols, rows), _addr(addr), _cols(cols), _rows(rows) {}

bool Display::begin() {
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0) {
    Serial.println(F("LCD not found"));
    return false;
  }

  _lcd.init();
  _lcd.backlight();
  _ready = true;
  Serial.println(F("LCD initialized"));
  return true;
}

void Display::clear() {
  if (!_ready) return;
  _lcd.clear();
}

void Display::print(uint8_t col, uint8_t row, std::string_view text) {
  if (!_ready) return;
  _lcd.setCursor(col, row);
  for (char c : text) _lcd.write(c);
}

void Display::printCenter(uint8_t row, std::string_view text) {
  if (!_ready) return;
  uint8_t col = (text.length() < _cols) ? (_cols - text.length()) / 2 : 0;
  _lcd.setCursor(col, row);
  for (char c : text) _lcd.write(c);
}

void Display::showStartup() {
  clear();
  printCenter(1, "Smart Server");
  printCenter(2, "Starting...");
}

void Display::showApMode(std::string_view ip) {
  clear();
  printCenter(0, "SETUP MODE");
  printCenter(1, "Connect AP:");
  printCenter(2, "TempMonitor-Setup");
  printCenter(3, ip);
}

void Display::showError(std::string_view message) {
  clear();
  printCenter(1, "ERROR");
  printCenter(2, message);
}

void Display::setWifiInfo(bool connected, const String& ip) {
  _wifiConnected = connected;
  _ipAddress = ip;
}

void Display::setTelemetry(float temperature, float humidity, bool valid,
                           bool fan1On, bool fan2On, bool warning) {
  _temperature = temperature;
  _humidity = humidity;
  _sensorValid = valid;
  _fan1On = fan1On;
  _fan2On = fan2On;
  _warning = warning;
}

void Display::setSecurity(const String& doorState, const String& accessMessage,
                          bool lockoutActive, uint32_t lockoutRemainSec) {
  _doorState = doorState;
  _accessMessage = accessMessage;
  _lockoutActive = lockoutActive;
  _lockoutRemainSec = lockoutRemainSec;
}

void Display::renderHeaderScroll() {
  if (!_ready) return;

  String text;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", &timeinfo);
    text += timeStr;
  } else {
    text += "SYNCING TIME";
  }

  text += " | IP:";
  text += (_wifiConnected ? _ipAddress : "DISCONNECTED");
  text += " | ";

  if (text.length() <= _cols) {
    _lcd.setCursor(0, 0);
    _lcd.print(text);
    for (size_t i = text.length(); i < _cols; ++i) _lcd.write(' ');
    return;
  }

  String row;
  row.reserve(_cols);
  for (uint8_t i = 0; i < _cols; ++i) {
    row += text[(_scrollOffset + i) % text.length()];
  }

  _lcd.setCursor(0, 0);
  _lcd.print(row);
  _scrollOffset = (_scrollOffset + 1) % text.length();
}

void Display::showMainScreen() {
  if (!_ready) return;

  renderHeaderScroll();

  char row1[32];
  if (_sensorValid) {
    snprintf(row1, sizeof(row1), "T:%4.1fC H:%4.1f%%", static_cast<double>(_temperature),
             static_cast<double>(_humidity));
  } else {
    snprintf(row1, sizeof(row1), "T:---- H:----");
  }
  _lcd.setCursor(0, 1);
  _lcd.print(row1);
  for (size_t i = strlen(row1); i < _cols; ++i) _lcd.write(' ');

  String row2 = "F1:";
  row2 += _fan1On ? "ON " : "OFF";
  row2 += " F2:";
  row2 += _fan2On ? "ON " : "OFF";
  row2 += " ";
  row2 += _warning ? "WARN" : "NORM";
  _lcd.setCursor(0, 2);
  _lcd.print(row2);
  for (size_t i = row2.length(); i < _cols; ++i) _lcd.write(' ');

  String row3 = "D:";
  row3 += _doorState;
  row3 += " ";
  if (_lockoutActive) {
    row3 += "LCK ";
    row3 += String(_lockoutRemainSec);
    row3 += "s";
  } else {
    row3 += _accessMessage;
  }
  if (row3.length() > _cols) row3 = row3.substring(0, _cols);
  _lcd.setCursor(0, 3);
  _lcd.print(row3);
  for (size_t i = row3.length(); i < _cols; ++i) _lcd.write(' ');
}

void Display::loop() {
  if (!_ready) return;
  if (millis() - _lastScrollTime < SCROLL_INTERVAL) return;
  _lastScrollTime = millis();
  showMainScreen();
}
