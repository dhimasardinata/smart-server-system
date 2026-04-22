#pragma once

#include "Config.h"

#include <Arduino.h>
#include <Keypad.h>
#include <deque>
#include <memory>

#ifdef CLOSED
#undef CLOSED
#endif
#ifdef OPEN
#undef OPEN
#endif

enum class AccessEventType : uint8_t {
  // Jenis kejadian saat akses dipakai.
  // Dipakai supaya layar dan catatan tahu apa yang baru terjadi.
  AccessGranted,
  AccessDenied,
  LockoutStarted,
  LockoutEnded
};

// Satu catatan kecil tentang kejadian saat pintu dipakai.
// Isinya dipakai untuk layar, catatan, dan pengiriman ke cloud.
struct AccessEvent {
  // Jenis kejadian yang sedang dicatat.
  AccessEventType type = AccessEventType::AccessDenied;
  // Nama pengguna yang memicu kejadian ini.
  String userId;
  // Nama tampil yang lebih enak dibaca orang.
  String displayName;
  // Hasil singkat seperti GRANTED atau DENIED.
  String result;
  // Alasan singkat kenapa kejadian ini muncul.
  String reason;
  // Jumlah salah masuk PIN sebelum kejadian ini.
  uint8_t failedCount = 0;
  // Waktu kapan penguncian berakhir.
  uint32_t lockoutUntilEpoch = 0;
};

// Hasil pemeriksaan PIN.
// Dipakai untuk menentukan apakah pintu dibuka atau tidak.
struct AuthResult {
  // Tanda bahwa PIN cocok.
  bool success = false;
  // Tanda bahwa pengguna ini admin.
  bool isAdmin = false;
  // Nama pengguna yang cocok.
  String userId;
  // Nama tampil untuk layar.
  String displayName;
};

class AccessController {
 public:
  // Menyiapkan keypad dan mengambil pengaturan awal.
  void begin(ConfigManager* config);
  // Perbarui keadaan yang berubah dari waktu ke waktu.
  void update();

  // Ambil tombol yang sedang ditekan sekarang.
  char getKey();
  // Periksa PIN yang dimasukkan pengguna.
  AuthResult validatePin(const String& pin);
  // Ganti PIN salah satu pengguna.
  bool changePin(const String& userId, const String& newPin, String& error);
  // Buat nama pengguna baru yang belum dipakai.
  String generateUserId() const;

  // Cek apakah keypad sedang dikunci sementara.
  [[nodiscard]] bool isLockoutActive() const;
  // Hitung sisa waktu penguncian.
  [[nodiscard]] uint32_t lockoutRemainingSec() const;
  // Lihat jumlah percobaan gagal.
  [[nodiscard]] uint8_t failedAttempts() const { return _failedAttempts; }
  // Pesan terakhir yang ditampilkan ke layar.
  [[nodiscard]] const String& lastMessage() const { return _lastMessage; }
  // Isi PIN yang sedang diketik.
  [[nodiscard]] const String& inputBuffer() const { return _pinBuffer; }

  // Ambil satu kejadian dari antrian catatan.
  bool popEvent(AccessEvent& outEvent);
  // Ambil tanda bahwa pintu perlu dibuka.
  bool consumeUnlockRequest();

  // Tambah atau ubah pengguna dari menu admin.
  bool upsertUser(const String& userId, const String& displayName,
                  const String& pin, bool enabled, String& error);

  // Beri akses baca ke pengatur utama.
  ConfigManager* config() const { return _config; }

 private:
  // Pengaturan utama dibaca dari sini.
  ConfigManager* _config = nullptr;
  // Objek keypad yang dibentuk saat begin().
  std::unique_ptr<Keypad> _keypad;
  // Catatan kejadian akses yang belum dibaca.
  std::deque<AccessEvent> _events;

  // Buffer PIN yang sedang diketik.
  String _pinBuffer;
  // Pesan terakhir yang dipakai layar.
  String _lastMessage = "READY";
  // Jumlah salah PIN yang sedang dihitung.
  uint8_t _failedAttempts = 0;
  // Waktu sampai kapan keypad terkunci.
  unsigned long _lockoutUntilMs = 0;
  // Penanda apakah sebelumnya sedang terkunci.
  bool _lockoutWasActive = false;
  // Penanda bahwa pintu perlu dibuka setelah PIN valid.
  bool _unlockRequested = false;

  // Masukkan satu catatan ke antrian.
  void pushEvent(const AccessEvent& event);
};
