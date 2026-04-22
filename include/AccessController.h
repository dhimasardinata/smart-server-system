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
  // Kejadian saat masuk: diterima, ditolak, dikunci, dibuka lagi.
  AccessGranted,
  AccessDenied,
  LockoutStarted,
  LockoutEnded
};

// Tempat menyimpan catatan kejadian saat pintu dipakai.
struct AccessEvent {
  AccessEventType type = AccessEventType::AccessDenied;
  String userId;
  String displayName;
  String result;
  String reason;
  uint8_t failedCount = 0;
  uint32_t lockoutUntilEpoch = 0;
};

// Hasil cek PIN untuk menentukan langkah berikutnya.
struct AuthResult {
  bool success = false;
  bool isAdmin = false;
  String userId;
  String displayName;
};

class AccessController {
 public:
  // Menyiapkan keypad dan mengambil pengaturan.
  void begin(ConfigManager* config);
  void update();

  char getKey();
  AuthResult validatePin(const String& pin);
  bool changePin(const String& userId, const String& newPin, String& error);
  String generateUserId() const;

  [[nodiscard]] bool isLockoutActive() const;
  [[nodiscard]] uint32_t lockoutRemainingSec() const;
  [[nodiscard]] uint8_t failedAttempts() const { return _failedAttempts; }
  [[nodiscard]] const String& lastMessage() const { return _lastMessage; }
  [[nodiscard]] const String& inputBuffer() const { return _pinBuffer; }

  bool popEvent(AccessEvent& outEvent);
  bool consumeUnlockRequest();

  // Tambah atau ubah pengguna dari menu admin.
  bool upsertUser(const String& userId, const String& displayName,
                  const String& pin, bool enabled, String& error);

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
  uint8_t _failedAttempts = 0;
  unsigned long _lockoutUntilMs = 0;
  bool _lockoutWasActive = false;
  bool _unlockRequested = false;

  void pushEvent(const AccessEvent& event);
};
