#include "Display.h"
#include "Config.h"
#include "I2CBus.h"

#include <time.h>

Display::Display(uint8_t addr, uint8_t cols, uint8_t rows)
    : _lcd(addr, cols, rows), _addr(addr), _cols(cols), _rows(rows) {}

bool Display::initialize(bool recoveredLog) {
  // Cek dulu apakah layar benar-benar ada di jalurnya.
  // Kalau tidak ada, jangan lanjut karena semua tulisan akan gagal.
  if (!I2CBus::probe(_addr)) {
    // Kalau tidak ada, tandai belum siap.
    _ready = false;
    return false;
  }
  // Kalau ditemukan, hidupkan layar dan lampunya.
  _lcd.init();
  _lcd.backlight();
  _ready = true;
  _lastPresenceCheckMs = millis();
  _lastScrollTime = 0;
  if (recoveredLog) {
    Serial.println(F("LCD reinitialized"));
    _recoveredSinceLastCheck = true;
  } else {
    Serial.println(F("LCD initialized"));
  }
  return true;
}

bool Display::begin() {
  // Saat awal nyala, layar langsung dicoba hidupkan.
  if (!initialize(false)) {
    Serial.println(F("LCD not found"));
    return false;
  }
  return true;
}

void Display::scheduleRecovery(unsigned long delayMs) {
  // Tandai kapan layar boleh dicoba lagi.
  // Ini dipakai kalau layar sempat terganggu setelah relay berubah.
  _scheduledRecoveryAtMs = millis() + delayMs;
  _recoveryScheduled = true;
}

bool Display::maintainConnection() {
  const unsigned long now = millis();

  // Kalau ada perbaikan yang sudah dijadwalkan, jalankan saat waktunya tiba.
  // Dengan cara ini, layar bisa pulih sendiri kalau sempat kacau.
  if (_recoveryScheduled &&
      static_cast<long>(now - _scheduledRecoveryAtMs) >= 0) {
    _recoveryScheduled = false;
    if (!I2CBus::recover("LCD scheduled")) {
      Serial.println(F("I2C: LCD bus still held during scheduled recovery"));
    }
    if (!initialize(true)) {
      Serial.println(F("LCD scheduled recovery failed"));
      return false;
    }
    return true;
  }

  if (_ready) {
    // Jangan terlalu sering mengecek layar supaya jalurnya tidak sibuk.
    if (now - _lastPresenceCheckMs < PRESENCE_CHECK_MS) {
      return true;
    }
    // Catat waktu cek terakhir.
    _lastPresenceCheckMs = now;
    // Kalau masih responsif, lanjut seperti biasa.
    if (I2CBus::probe(_addr)) {
      return true;
    }
    // Kalau tidak responsif, tandai tidak siap.
    _ready = false;
    Serial.println(F("LCD disconnected, waiting for recovery"));
  }

  if (now - _lastReconnectAttemptMs < RECONNECT_INTERVAL_MS) {
    return false;
  }
  _lastReconnectAttemptMs = now;

  // Jika layar tidak merespons, coba perbaiki jalurnya lalu hidupkan lagi.
  if (!I2CBus::probe(_addr)) {
    if (!I2CBus::recover("LCD")) {
      Serial.println(F("I2C: LCD bus still held after recovery"));
    }
  }

  return initialize(true);
}

bool Display::consumeRecovered() {
  const bool recovered = _recoveredSinceLastCheck;
  _recoveredSinceLastCheck = false;
  return recovered;
}

void Display::clear() {
  if (!_ready) return;
  // Hapus semua tulisan di layar.
  // Ini dipakai sebelum menampilkan halaman baru.
  _lcd.clear();
}

void Display::clearRow(uint8_t row) {
  if (!_ready) return;
  // Geser kursor ke awal baris.
  _lcd.setCursor(0, row);
  // Isi baris dengan spasi agar tulisan lama hilang.
  for (uint8_t i = 0; i < _cols; ++i) _lcd.write(' ');
}

void Display::print(uint8_t col, uint8_t row, std::string_view text) {
  if (!_ready) return;
  // Posisikan kursor di kolom dan baris yang diminta.
  // Setelah itu, tulis isi teks ke layar.
  _lcd.setCursor(col, row);
  // Tulis isi teks satu karakter per satu karakter.
  for (char c : text) _lcd.write(c);
}

void Display::printCenter(uint8_t row, std::string_view text) {
  if (!_ready) return;
  // Hitung posisi agar teks berada di tengah.
  // Ini dipakai untuk judul atau pesan singkat.
  uint8_t col = (text.length() < _cols) ? (_cols - text.length()) / 2 : 0;
  // Pindahkan kursor ke posisi tengah.
  _lcd.setCursor(col, row);
  // Tulis isi teks ke layar.
  for (char c : text) _lcd.write(c);
}

void Display::printRow(uint8_t row, const char* text) {
  if (!_ready) return;
  // Mulai dari awal baris.
  _lcd.setCursor(0, row);
  // Cari panjang teks agar baris bisa dipenuhi sampai lebar layar.
  size_t len = strlen(text);
  for (size_t i = 0; i < _cols; ++i) {
    // Tulis karakter asli atau spasi jika teks sudah habis.
    _lcd.write(i < len ? text[i] : ' ');
  }
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

  // Susun baris atas dari waktu dan IP.
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

  // Kalau teks muat, tampilkan langsung.
  if (text.length() <= _cols) {
    // Kalau muat, tampilkan langsung tanpa geser.
    _lcd.setCursor(0, 0);
    _lcd.print(text);
    for (size_t i = text.length(); i < _cols; ++i) _lcd.write(' ');
    return;
  }

  String row;
  row.reserve(_cols);
  // Kalau kepanjangan, geser pelan-pelan.
  for (uint8_t i = 0; i < _cols; ++i) {
    row += text[(_scrollOffset + i) % text.length()];
  }

  _lcd.setCursor(0, 0);
  _lcd.print(row);
  _scrollOffset = (_scrollOffset + 1) % text.length();
}

void Display::showMainScreen() {
  if (!_ready) return;

  // Tulis baris atas yang bisa bergerak pelan.
  // Bagian ini menampilkan waktu dan alamat jaringan.
  renderHeaderScroll();

  char row1[32];
  if (_sensorValid) {
    // Tampilkan suhu dan kelembapan kalau data valid.
    // Kalau tidak valid, isi dibuat kosong agar tidak menyesatkan.
    snprintf(row1, sizeof(row1), "T:%4.1fC H:%4.1f%%", static_cast<double>(_temperature),
             static_cast<double>(_humidity));
  } else {
    // Kalau sensor belum valid, tampilkan tanda kosong.
    snprintf(row1, sizeof(row1), "T:---- H:----");
  }
  printRow(1, row1);

  char row2[24];
  // Baris ini merangkum kondisi kipas dan tanda bahaya.
  // Jadi orang bisa lihat keadaan alat hanya dari satu baris.
  snprintf(row2, sizeof(row2), "F1:%s F2:%s %s",
           _fan1On ? "ON " : "OFF", _fan2On ? "ON " : "OFF",
           _warning ? "WARN" : "NORM");
  printRow(2, row2);

  char row3[24];
  if (_lockoutActive) {
    // Kalau terkunci, tampilkan sisa waktu.
    snprintf(row3, sizeof(row3), "D:%.8s LCK %lus",
             _doorState.c_str(), static_cast<unsigned long>(_lockoutRemainSec));
  } else {
    // Kalau tidak terkunci, tampilkan status pintu biasa.
    snprintf(row3, sizeof(row3), "D:%-8.8s %.12s",
             _doorState.c_str(), _accessMessage.c_str());
  }
  if (strlen(row3) > _cols) row3[_cols] = '\0';
  printRow(3, row3);
}

void Display::showPinEntry(uint8_t pinLen, bool lockout, uint32_t lockSec) {
  if (!_ready) return;
  clear();
  // Judul layar input PIN.
  printCenter(0, "== MASUKKAN PIN ==");

  if (lockout) {
    char buf[24];
    // Tampilkan hitung mundur saat terkunci.
    snprintf(buf, sizeof(buf), "TERKUNCI %lud", static_cast<unsigned long>(lockSec));
    printCenter(2, buf);
    return;
  }

  char pinStr[16] = "PIN: ";
  size_t offset = 5;
  // Ganti tiap digit dengan bintang supaya orang lain tidak melihat PIN.
  for (uint8_t i = 0; i < pinLen && offset < 14; ++i) pinStr[offset++] = '*';
  pinStr[offset] = '\0';
  printRow(2, pinStr);
  // Tampilkan petunjuk tombol di baris bawah.
  printRow(3, "D=Hps *=Btl #=OK");
}

void Display::showUnlockOk(const String& name) {
  if (!_ready) return;
  clear();
  printCenter(0, "== AKSES DITERIMA ==");
  printCenter(2, "Selamat datang,");
  String truncated = name.substring(0, _cols);
  printCenter(3, std::string_view(truncated.c_str(), truncated.length()));
}

void Display::showAdminMenu() {
  if (!_ready) return;
  clear();
  printRow(0, "== MENU ADMIN ==");
  printRow(1, "1.Pintu  2.GantiPIN");
  printRow(2, "3.TambahUsr 4.Hapus");
  printRow(3, "[*] Kembali");
}

void Display::showUserList(const UserCredential* users, size_t maxUsers, uint8_t action) {
  if (!_ready) return;
  clear();

  const char* title = action == 0 ? "== GANTI PIN ==" : "== HAPUS USER ==";
  printRow(0, title);

  uint8_t slot = 1;
  uint8_t row = 1;
  for (size_t i = 0; i < maxUsers && row <= 2; ++i) {
    if (users[i].userId.length() == 0 || !users[i].enabled) continue;
    if (action == 1 && i == 0) continue;

    char buf[24];
    String uid = users[i].userId.substring(0, 8);
    snprintf(buf, sizeof(buf), "%d.%-9s", slot, uid.c_str());

    if (slot % 2 == 1) {
      _lcd.setCursor(0, row);
      _lcd.print(buf);
    } else {
      _lcd.setCursor(10, row);
      _lcd.print(buf);
      ++row;
    }
    ++slot;
  }

  printRow(3, "[*] Batal");
}

void Display::showChangePin(const String& userId, uint8_t step, uint8_t len) {
  if (!_ready) return;
  clear();

  char title[24];
  String uid = userId.substring(0, 10);
  snprintf(title, sizeof(title), "PIN: %s", uid.c_str());
  printRow(0, title);

  if (step == 0) {
    char buf[24] = "PIN Baru: ";
    size_t offset = 10;
    for (uint8_t i = 0; i < len && offset < 19; ++i) buf[offset++] = '*';
    buf[offset] = '\0';
    printRow(1, buf);
    clearRow(2);
  } else {
    printRow(1, "PIN Baru: OK");
    char buf[24] = "Konfirmasi: ";
    size_t offset = 12;
    for (uint8_t i = 0; i < len && offset < 19; ++i) buf[offset++] = '*';
    buf[offset] = '\0';
    printRow(2, buf);
  }

  printRow(3, "D=Hps *=Btl #=OK");
}

void Display::showAddUser(const String& autoId, uint8_t pinLen) {
  if (!_ready) return;
  clear();
  printRow(0, "== TAMBAH USER ==");

  char idRow[24];
  snprintf(idRow, sizeof(idRow), "ID: %s", autoId.c_str());
  printRow(1, idRow);

  char pinRow[24] = "PIN: ";
  size_t offset = 5;
  for (uint8_t i = 0; i < pinLen && offset < 14; ++i) pinRow[offset++] = '*';
  pinRow[offset] = '\0';
  printRow(2, pinRow);

  printRow(3, "D=Hps *=Btl #=Sav");
}

void Display::showConfirmDelete(const String& userId) {
  if (!_ready) return;
  clear();
  printRow(0, "== HAPUS USER? ==");

  char buf[24];
  snprintf(buf, sizeof(buf), "User: %s", userId.substring(0, 12).c_str());
  printRow(1, buf);
  printRow(2, "1.Ya  [*] Batal");
  clearRow(3);
}

void Display::showMessage(const char* title, const char* msg, bool success) {
  if (!_ready) return;
  clear();
  printCenter(0, title);
  printCenter(2, msg);
  printRow(3, success ? "OK" : "GAGAL");
}

void Display::loop() {
  // Putaran layar hanya jalan jika layar masih siap.
  if (!maintainConnection()) return;
  if (millis() - _lastScrollTime < SCROLL_INTERVAL) return;
  _lastScrollTime = millis();
  showMainScreen();
}
