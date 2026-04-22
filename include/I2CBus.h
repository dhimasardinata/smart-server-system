#pragma once

#include <Arduino.h>

namespace I2CBus {

// Siapkan jalur bersama untuk dua alat.
void begin();
// Cek apakah alat di jalur itu merespons.
[[nodiscard]] bool probe(uint8_t address);
// Coba hidupkan ulang jalurnya kalau sempat macet.
[[nodiscard]] bool recover(const char* context = nullptr);

}  // akhir bagian ini
