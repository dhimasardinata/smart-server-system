#pragma once

#include <Arduino.h>

namespace I2CBus {

// Siapkan jalur bersama untuk dua alat.
// Jalur ini dipakai oleh sensor dan layar LCD.
void begin();
// Cek apakah alat di jalur itu merespons.
// Dipakai untuk memastikan perangkat benar-benar ada.
[[nodiscard]] bool probe(uint8_t address);
// Coba hidupkan ulang jalurnya kalau sempat macet.
// Ini membantu kalau kabel atau alat sempat menggantung.
[[nodiscard]] bool recover(const char* context = nullptr);

}  // akhir bagian ini
