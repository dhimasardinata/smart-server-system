#pragma once

#include <Arduino.h>

namespace I2CBus {

void begin();
[[nodiscard]] bool probe(uint8_t address);
[[nodiscard]] bool recover(const char* context = nullptr);

}  // namespace I2CBus
