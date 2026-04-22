#pragma once

#include <cstdint>

namespace Pins {

// Jalur untuk dua alat.
constexpr uint8_t SDA = 21;
constexpr uint8_t SCL = 22;

// Nomor alamat LCD dan ukuran layarnya.
constexpr uint8_t I2C_ADDR_LCD = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

// Pin output untuk beban utama.
constexpr uint8_t RELAY_FAN1 = 4;
constexpr uint8_t RELAY_FAN2 = 18;
constexpr uint8_t RELAY_SOLENOID = 19;
constexpr uint8_t RELAY_ALERT = 23;

// Aturan hidup-mati untuk saklar.
constexpr bool RELAY_ACTIVE_LOW = false;
constexpr bool SOLENOID_ACTIVE_LOW = false;
constexpr bool ALERT_ACTIVE_LOW = false;

// Layout keypad 4x4.
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
