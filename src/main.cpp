#include "App.h"

#include <Arduino.h>

// Objek utama aplikasi. Semua alur sistem dikendalikan lewat instance ini.
App app;

// setup() hanya dipanggil sekali saat board baru menyala.
void setup() {
  // Serahkan semua persiapan ke objek App.
  app.setup();
}

// loop() berjalan terus-menerus selama ESP32 hidup.
void loop() {
  // Serahkan semua kerja berulang ke objek App.
  app.loop();
}
