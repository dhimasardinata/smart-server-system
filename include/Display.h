#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <string_view>

struct UserCredential;

class Display {
 public:
  // Pengurus layar LCD.
  // Semua teks yang muncul di panel depan lewat kelas ini.
  Display(uint8_t addr, uint8_t cols, uint8_t rows);

  // Menyalakan layar dan memastikan jalur komunikasinya siap.
  // Kalau gagal, layar dianggap belum tersedia.
  bool begin();
  bool maintainConnection();
  void scheduleRecovery(unsigned long delayMs = 30);
  void clear();
  void loop();

  void print(uint8_t col, uint8_t row, std::string_view text);
  void printCenter(uint8_t row, std::string_view text);
  void printRow(uint8_t row, const char* text);

  void showStartup();
  void showApMode(std::string_view ip);
  void showError(std::string_view message);
  // Layar utama menampilkan suhu, kelembapan, kipas, dan pintu.
  void showMainScreen();

  // Layar input PIN dengan penanda jumlah angka yang sudah diketik.
  void showPinEntry(uint8_t pinLen, bool lockout = false, uint32_t lockSec = 0);
  // Layar sukses saat pintu berhasil dibuka.
  void showUnlockOk(const String& name);
  // Menu utama admin.
  void showAdminMenu();
  // Daftar pengguna yang bisa dipilih.
  void showUserList(const UserCredential* users, size_t maxUsers, uint8_t action);
  // Layar ganti PIN.
  void showChangePin(const String& userId, uint8_t step, uint8_t len);
  // Layar tambah pengguna.
  void showAddUser(const String& autoId, uint8_t pinLen);
  // Layar konfirmasi hapus.
  void showConfirmDelete(const String& userId);
  // Pesan singkat untuk berhasil atau gagal.
  void showMessage(const char* title, const char* msg, bool success);

  // Salin data terbaru ke layar.
  // Data ini disimpan dulu sebelum digambar.
  void setWifiInfo(bool connected, const String& ip);
  void setTelemetry(float temperature, float humidity, bool valid, bool fan1On,
                    bool fan2On, bool warning);
  void setSecurity(const String& doorState, const String& accessMessage,
                   bool lockoutActive, uint32_t lockoutRemainSec);
  bool consumeRecovered();

  // Cek apakah layar sudah siap dipakai.
  [[nodiscard]] bool isReady() const { return _ready; }

 private:
  // Objek LCD dari pustaka.
  // Inilah yang benar-benar berbicara ke layar fisik.
  LiquidCrystal_I2C _lcd;
  // Alamat I2C layar.
  uint8_t _addr;
  // Lebar layar.
  uint8_t _cols;
  // Tinggi layar.
  uint8_t _rows;
  // Penanda apakah layar siap dipakai.
  bool _ready = false;

  // Data terbaru yang akan ditampilkan.
  // Nilai ini diisi dari sensor, WiFi, dan akses.
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
  unsigned long _lastPresenceCheckMs = 0;
  unsigned long _lastReconnectAttemptMs = 0;
  unsigned long _scheduledRecoveryAtMs = 0;
  bool _recoveredSinceLastCheck = false;
  bool _recoveryScheduled = false;

  // Status scrolling dan pemulihan koneksi.
  // Dipakai agar tampilan tetap hidup dan tidak macet.
  unsigned long _lastScrollTime = 0;
  int _scrollOffset = 0;
  static constexpr unsigned long SCROLL_INTERVAL = 350;
  static constexpr unsigned long PRESENCE_CHECK_MS = 2000;
  static constexpr unsigned long RECONNECT_INTERVAL_MS = 1000;

  // Gambar bagian atas yang bisa bergerak pelan.
  void renderHeaderScroll();
  // Bersihkan satu baris saja.
  void clearRow(uint8_t row);
  // Coba hidupkan layar dengan ulang.
  bool initialize(bool recoveredLog);
};
