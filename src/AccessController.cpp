#include "AccessController.h"

#include "PinMap.h"

#include <mbedtls/sha256.h>
#include <time.h>

namespace {
constexpr size_t PIN_MAX_LEN = 8;
constexpr size_t PIN_MIN_LEN = 4;

String hashPinSha256(const String& pin) {
  // PIN tidak disimpan apa adanya, tapi diubah jadi kode pengaman.
  // Jadi kalau memori dibaca orang lain, PIN asli tidak langsung kelihatan.
  uint8_t hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(
      &ctx, reinterpret_cast<const unsigned char*>(pin.c_str()), pin.length());
  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  char out[65];
  for (size_t i = 0; i < 32; ++i) {
    snprintf(&out[i * 2], 3, "%02x", hash[i]);
  }
  out[64] = '\0';
  return String(out);
}

bool isValidPinFormat(const String& pin) {
  // PIN harus angka semua dan panjangnya sesuai ketentuan.
  // Aturan ini dipakai supaya input lebih rapi dan mudah dicek.
  if (pin.length() < PIN_MIN_LEN || pin.length() > PIN_MAX_LEN) return false;
  for (size_t i = 0; i < pin.length(); ++i) {
    if (!isDigit(pin[i])) return false;
  }
  return true;
}
}  // namespace

void AccessController::begin(ConfigManager* config) {
  _config = config;

  // Susunan tombol disiapkan sekali agar langsung bisa dipakai.
  // Bagian ini hanya menyiapkan bentuk keypad, bukan membaca tombolnya.
  static char keymap[Pins::KEYPAD_ROWS][Pins::KEYPAD_COLS] = {
      {Pins::KEYPAD_MAP[0][0], Pins::KEYPAD_MAP[0][1], Pins::KEYPAD_MAP[0][2],
       Pins::KEYPAD_MAP[0][3]},
      {Pins::KEYPAD_MAP[1][0], Pins::KEYPAD_MAP[1][1], Pins::KEYPAD_MAP[1][2],
       Pins::KEYPAD_MAP[1][3]},
      {Pins::KEYPAD_MAP[2][0], Pins::KEYPAD_MAP[2][1], Pins::KEYPAD_MAP[2][2],
       Pins::KEYPAD_MAP[2][3]},
      {Pins::KEYPAD_MAP[3][0], Pins::KEYPAD_MAP[3][1], Pins::KEYPAD_MAP[3][2],
       Pins::KEYPAD_MAP[3][3]},
  };
  static byte rowPins[Pins::KEYPAD_ROWS] = {
      static_cast<byte>(Pins::KEYPAD_ROW_PINS[0]),
      static_cast<byte>(Pins::KEYPAD_ROW_PINS[1]),
      static_cast<byte>(Pins::KEYPAD_ROW_PINS[2]),
      static_cast<byte>(Pins::KEYPAD_ROW_PINS[3]),
  };
  static byte colPins[Pins::KEYPAD_COLS] = {
      static_cast<byte>(Pins::KEYPAD_COL_PINS[0]),
      static_cast<byte>(Pins::KEYPAD_COL_PINS[1]),
      static_cast<byte>(Pins::KEYPAD_COL_PINS[2]),
      static_cast<byte>(Pins::KEYPAD_COL_PINS[3]),
  };

  _keypad = std::make_unique<Keypad>(makeKeymap(keymap), rowPins, colPins,
                                     Pins::KEYPAD_ROWS, Pins::KEYPAD_COLS);
}

char AccessController::getKey() {
  // Kalau keypad belum ada, berarti belum bisa baca tombol.
  if (_keypad == nullptr) return NO_KEY;
  return _keypad->getKey();
}

bool AccessController::upsertUser(const String& userId, const String& displayName,
                                  const String& pin, bool enabled,
                                  String& error) {
  error = "";
  // Semua perubahan pengguna harus melewati pengatur utama.
  if (_config == nullptr) {
    error = "controller not initialized";
    return false;
  }
  if (userId.length() == 0) {
    // Nama pengguna tidak boleh kosong.
    error = "userId required";
    return false;
  }
  if (!isValidPinFormat(pin)) {
    // PIN yang salah bentuk langsung ditolak.
    error = "pin must be 4-8 numeric digits";
    return false;
  }

  UserCredential user;
  user.userId = userId;
  // Kalau nama tampil kosong, pakai nama pengguna sebagai gantinya.
  user.displayName = displayName.length() > 0 ? displayName : userId;
  // Simpan PIN dalam bentuk aman.
  user.pinHash = hashPinSha256(pin);
  user.enabled = enabled;
  if (!_config->upsertUser(user)) {
    error = "failed to save user";
    return false;
  }
  return true;
}

AuthResult AccessController::validatePin(const String& pin) {
  AuthResult result;
  // Kalau pengatur belum siap, PIN tidak bisa diperiksa.
  if (_config == nullptr || !isValidPinFormat(pin)) return result;

  // Kalau cocok dengan salah satu pengguna aktif, akses diterima.
  // Periksa satu per satu sampai ada yang cocok.
  const String hash = hashPinSha256(pin);
  for (const auto& user : _config->data.users) {
    if (!user.enabled || user.userId.length() == 0) continue;
    if (user.pinHash == hash) {
      // PIN cocok, jadi akses boleh lanjut.
      result.success = true;
      result.userId = user.userId;
      result.displayName = user.displayName;
      // Admin adalah pengguna pertama yang tersimpan.
      result.isAdmin = (user.userId == _config->data.users[0].userId);

      // Hitungan salah PIN direset kalau berhasil.
      _failedAttempts = 0;
      // Pesan singkat untuk layar.
      _lastMessage = "ACCESS GRANTED";
      // Tandai bahwa pintu harus dibuka.
      _unlockRequested = true;

      // Simpan catatan supaya layar dan cloud tahu.
      AccessEvent event;
      event.type = AccessEventType::AccessGranted;
      event.userId = user.userId;
      event.displayName = user.displayName;
      event.result = "GRANTED";
      event.reason = "VALID_PIN";
      event.failedCount = 0;
      pushEvent(event);
      return result;
    }
  }

  _failedAttempts++;
  // Kalau salah, pesan layar ikut berubah.
  _lastMessage = "ACCESS DENIED";

  // Simpan percobaan gagal untuk menghitung batas berikutnya.
  // Catatan ini dipakai untuk layar dan spreadsheet.
  AccessEvent denied;
  denied.type = AccessEventType::AccessDenied;
  denied.result = "DENIED";
  denied.reason = "INVALID_PIN";
  denied.failedCount = _failedAttempts;
  pushEvent(denied);

  if (_config != nullptr && _failedAttempts >= _config->data.maxFailedAttempts) {
    // Kalau salah terlalu banyak, keypad dikunci sementara.
    _lockoutUntilMs = millis() + (_config->data.keypadLockoutSec * 1000UL);
    _failedAttempts = 0;
    // Layar menampilkan keadaan penguncian.
    _lastMessage = "LOCKOUT ACTIVE";

    // Catatan ini menandai awal penguncian.
    AccessEvent lockout;
    lockout.type = AccessEventType::LockoutStarted;
    lockout.result = "LOCKOUT";
    lockout.reason = "MAX_FAILED_ATTEMPTS";
    lockout.failedCount = _config->data.maxFailedAttempts;
    lockout.lockoutUntilEpoch = static_cast<uint32_t>(time(nullptr)) +
                                _config->data.keypadLockoutSec;
    pushEvent(lockout);
  }

  return result;
}

bool AccessController::changePin(const String& userId, const String& newPin,
                                 String& error) {
  error = "";
  // PIN hanya bisa diganti kalau pengatur sudah siap.
  if (_config == nullptr) {
    error = "not initialized";
    return false;
  }
  if (!isValidPinFormat(newPin)) {
    // PIN baru juga harus mengikuti aturan.
    error = "PIN harus 4-8 digit angka";
    return false;
  }

  for (auto& user : _config->data.users) {
    if (user.userId == userId && user.enabled) {
      // Ganti PIN lama dengan PIN baru yang aman.
      user.pinHash = hashPinSha256(newPin);
      if (_config->save()) return true;
      error = "gagal menyimpan";
      return false;
    }
  }
  error = "user tidak ditemukan";
  return false;
}

String AccessController::generateUserId() const {
  // Buat nama pengguna otomatis yang belum dipakai.
  // Ini dipakai saat admin menambah pengguna baru.
  if (_config == nullptr) return "user01";
  for (int i = 1; i <= 99; ++i) {
    char buf[8];
    snprintf(buf, sizeof(buf), "user%02d", i);
    String candidate(buf);
    bool exists = false;
    for (const auto& user : _config->data.users) {
      if (user.userId == candidate) {
        exists = true;
        break;
      }
    }
    if (!exists) return candidate;
  }
  return "user99";
}

bool AccessController::consumeUnlockRequest() {
  // Jika belum ada permintaan buka, langsung gagal.
  if (!_unlockRequested) return false;
  _unlockRequested = false;
  return true;
}

bool AccessController::isLockoutActive() const {
  // Kondisi terkunci masih aktif kalau waktunya belum selesai.
  // Waktu ini dihitung dari millis() milik ESP32.
  return _lockoutUntilMs > millis();
}

uint32_t AccessController::lockoutRemainingSec() const {
  // Kalau tidak terkunci, sisa waktunya nol.
  if (!isLockoutActive()) return 0;
  return static_cast<uint32_t>((_lockoutUntilMs - millis()) / 1000UL);
}

void AccessController::pushEvent(const AccessEvent& event) {
  // Antrian dijaga agar tidak tumbuh tanpa batas.
  _events.push_back(event);
  if (_events.size() > 50) _events.pop_front();
}

bool AccessController::popEvent(AccessEvent& outEvent) {
  // Kalau antrian kosong, tidak ada yang perlu diambil.
  if (_events.empty()) return false;
  outEvent = _events.front();
  _events.pop_front();
  return true;
}

void AccessController::update() {
  // Saat penguncian selesai, kirim penanda agar tampilan ikut menyesuaikan.
  // Ini dijalankan terus supaya status tidak tertinggal.
  const bool lockoutNow = isLockoutActive();
  if (_lockoutWasActive && !lockoutNow) {
    AccessEvent event;
    event.type = AccessEventType::LockoutEnded;
    event.result = "INFO";
    event.reason = "LOCKOUT_ENDED";
    event.failedCount = _failedAttempts;
    pushEvent(event);
    _lastMessage = "LOCKOUT ENDED";
  }
  _lockoutWasActive = lockoutNow;
}
