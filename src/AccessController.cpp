#include "AccessController.h"

#include "PinMap.h"

#include <mbedtls/sha256.h>
#include <time.h>

namespace {
constexpr size_t PIN_MAX_LEN = 8;
constexpr size_t PIN_MIN_LEN = 4;

String hashPinSha256(const String& pin) {
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
  if (pin.length() < PIN_MIN_LEN || pin.length() > PIN_MAX_LEN) return false;
  for (size_t i = 0; i < pin.length(); ++i) {
    if (!isDigit(pin[i])) return false;
  }
  return true;
}
}  // namespace

void AccessController::begin(ConfigManager* config) {
  _config = config;

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

bool AccessController::upsertUser(const String& userId, const String& displayName,
                                  const String& pin, bool enabled,
                                  String& error) {
  error = "";
  if (_config == nullptr) {
    error = "controller not initialized";
    return false;
  }
  if (userId.length() == 0) {
    error = "userId required";
    return false;
  }
  if (!isValidPinFormat(pin)) {
    error = "pin must be 4-8 numeric digits";
    return false;
  }

  UserCredential user;
  user.userId = userId;
  user.displayName = displayName.length() > 0 ? displayName : userId;
  user.pinHash = hashPinSha256(pin);
  user.enabled = enabled;
  if (!_config->upsertUser(user)) {
    error = "failed to save user";
    return false;
  }
  return true;
}

bool AccessController::consumeUnlockRequest() {
  if (!_unlockRequested) return false;
  _unlockRequested = false;
  return true;
}

bool AccessController::isLockoutActive() const {
  return _lockoutUntilMs > millis();
}

uint32_t AccessController::lockoutRemainingSec() const {
  if (!isLockoutActive()) return 0;
  return static_cast<uint32_t>((_lockoutUntilMs - millis()) / 1000UL);
}

void AccessController::pushEvent(const AccessEvent& event) {
  _events.push_back(event);
  if (_events.size() > 50) _events.pop_front();
}

bool AccessController::popEvent(AccessEvent& outEvent) {
  if (_events.empty()) return false;
  outEvent = _events.front();
  _events.pop_front();
  return true;
}

void AccessController::update() {
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

  if (_keypad == nullptr) return;
  const char key = _keypad->getKey();
  if (key != NO_KEY) handleKey(key);
}

void AccessController::handleKey(char key) {
  if (isLockoutActive()) {
    _lastMessage = "LOCKED " + String(lockoutRemainingSec()) + "s";
    return;
  }

  if (key >= '0' && key <= '9') {
    if (_pinBuffer.length() < PIN_MAX_LEN) _pinBuffer += key;
    return;
  }

  if (key == '*') {
    _pinBuffer = "";
    _lastMessage = "INPUT CLEARED";
    return;
  }

  if (key == '#') submitPin();
}

bool AccessController::checkPin(const String& pin, UserCredential& matchedUser) const {
  if (_config == nullptr) return false;
  const String hash = hashPinSha256(pin);
  for (const auto& user : _config->data.users) {
    if (!user.enabled || user.userId.length() == 0) continue;
    if (user.pinHash == hash) {
      matchedUser = user;
      return true;
    }
  }
  return false;
}

void AccessController::submitPin() {
  if (!isValidPinFormat(_pinBuffer)) {
    _lastMessage = "PIN INVALID";
    _pinBuffer = "";
    return;
  }

  UserCredential matchedUser;
  if (checkPin(_pinBuffer, matchedUser)) {
    _failedAttempts = 0;
    _lastMessage = "ACCESS GRANTED";
    _unlockRequested = true;

    AccessEvent event;
    event.type = AccessEventType::AccessGranted;
    event.userId = matchedUser.userId;
    event.displayName = matchedUser.displayName;
    event.result = "GRANTED";
    event.reason = "VALID_PIN";
    event.failedCount = 0;
    pushEvent(event);
  } else {
    _failedAttempts++;
    _lastMessage = "ACCESS DENIED";

    AccessEvent denied;
    denied.type = AccessEventType::AccessDenied;
    denied.result = "DENIED";
    denied.reason = "INVALID_PIN";
    denied.failedCount = _failedAttempts;
    pushEvent(denied);

    if (_config != nullptr && _failedAttempts >= _config->data.maxFailedAttempts) {
      _lockoutUntilMs = millis() + (_config->data.keypadLockoutSec * 1000UL);
      _failedAttempts = 0;
      _lastMessage = "LOCKOUT ACTIVE";

      AccessEvent lockout;
      lockout.type = AccessEventType::LockoutStarted;
      lockout.result = "LOCKOUT";
      lockout.reason = "MAX_FAILED_ATTEMPTS";
      lockout.failedCount = _config->data.maxFailedAttempts;
      lockout.lockoutUntilEpoch = static_cast<uint32_t>(time(nullptr)) +
                                  _config->data.keypadLockoutSec;
      pushEvent(lockout);
    }
  }

  _pinBuffer = "";
}
