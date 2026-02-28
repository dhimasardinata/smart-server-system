#pragma once

#include <cstdint>

namespace Pins {

constexpr uint8_t SDA = 21;
constexpr uint8_t SCL = 22;

constexpr uint8_t I2C_ADDR_LCD = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

constexpr uint8_t RELAY_FAN1 = 4;
constexpr uint8_t RELAY_FAN2 = 18;
constexpr uint8_t RELAY_SOLENOID = 19;

constexpr bool RELAY_ACTIVE_LOW = true;

constexpr uint8_t KEYPAD_ROWS = 4;
constexpr uint8_t KEYPAD_COLS = 4;
constexpr uint8_t KEYPAD_ROW_PINS[KEYPAD_ROWS] = {32, 33, 25, 26};
constexpr uint8_t KEYPAD_COL_PINS[KEYPAD_COLS] = {27, 14, 12, 13};
constexpr char KEYPAD_MAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

}  // namespace Pins
