#include "I2CBus.h"

#include "PinMap.h"

#include <Wire.h>

namespace {
constexpr uint32_t I2C_CLOCK_HZ = 100000;
constexpr uint16_t I2C_TIMEOUT_MS = 50;
constexpr uint8_t RECOVERY_CLOCK_PULSES = 9;
constexpr uint32_t RECOVERY_STEP_US = 5;

void releaseLine(uint8_t pin) {
  // Tarik jalur ke keadaan aman tanpa memaksa tinggi/rendah terus-menerus.
  pinMode(pin, OUTPUT_OPEN_DRAIN);
  digitalWrite(pin, HIGH);
}

void pullLineLow(uint8_t pin) {
  // Tarik jalur ke rendah sebentar untuk memberi hentakan pemulihan.
  pinMode(pin, OUTPUT_OPEN_DRAIN);
  digitalWrite(pin, LOW);
}

void releaseBusPins() {
  // Kembalikan SDA dan SCL ke kondisi normal dengan pull-up.
  pinMode(Pins::SDA, INPUT_PULLUP);
  pinMode(Pins::SCL, INPUT_PULLUP);
}
}  // namespace

void I2CBus::begin() {
  // Mulai ulang jalur komunikasi dengan setelan yang sama.
  // Kecepatan standar dipilih supaya banyak alat lebih mudah cocok.
  Wire.end();
  Wire.begin(Pins::SDA, Pins::SCL);
  Wire.setClock(I2C_CLOCK_HZ);
  Wire.setTimeOut(I2C_TIMEOUT_MS);
}

bool I2CBus::probe(uint8_t address) {
  // Coba kirim sinyal kecil untuk melihat apakah alamat merespons.
  Wire.beginTransmission(address);
  return Wire.endTransmission(true) == 0;
}

bool I2CBus::recover(const char* context) {
  if (context != nullptr && context[0] != '\0') {
    Serial.printf("I2C: recovering bus for %s\n", context);
  } else {
    Serial.println(F("I2C: recovering bus"));
  }

  // Kalau jalur macet, kirim beberapa hentakan agar perangkat melepasnya.
  // Langkah ini sering dipakai saat sensor atau LCD menggantung.
  Wire.end();
  releaseBusPins();
  delayMicroseconds(RECOVERY_STEP_US);

  releaseLine(Pins::SCL);
  delayMicroseconds(RECOVERY_STEP_US);
  for (uint8_t i = 0;
       i < RECOVERY_CLOCK_PULSES && digitalRead(Pins::SDA) == LOW; ++i) {
    pullLineLow(Pins::SCL);
    delayMicroseconds(RECOVERY_STEP_US);
    releaseLine(Pins::SCL);
    delayMicroseconds(RECOVERY_STEP_US);
  }

  pullLineLow(Pins::SDA);
  delayMicroseconds(RECOVERY_STEP_US);
  releaseLine(Pins::SCL);
  delayMicroseconds(RECOVERY_STEP_US);
  releaseLine(Pins::SDA);
  delayMicroseconds(RECOVERY_STEP_US);
  releaseBusPins();

  // Kalau dua jalurnya kembali normal, berarti sudah lepas.
  // Setelah itu, jalur disiapkan lagi seperti semula.
  const bool busReleased =
      digitalRead(Pins::SDA) == HIGH && digitalRead(Pins::SCL) == HIGH;

  // Setelah itu, hidupkan lagi jalur komunikasi normal.
  begin();
  return busReleased;
}
