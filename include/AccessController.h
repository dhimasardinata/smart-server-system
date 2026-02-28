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
  AccessGranted,
  AccessDenied,
  LockoutStarted,
  LockoutEnded
};

struct AccessEvent {
  AccessEventType type = AccessEventType::AccessDenied;
  String userId;
  String displayName;
  String result;
  String reason;
  uint8_t failedCount = 0;
  uint32_t lockoutUntilEpoch = 0;
};

struct AuthResult {
  bool success = false;
  bool isAdmin = false;
  String userId;
  String displayName;
};

class AccessController {
 public:
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

  bool upsertUser(const String& userId, const String& displayName,
                  const String& pin, bool enabled, String& error);

  ConfigManager* config() const { return _config; }

 private:
  ConfigManager* _config = nullptr;
  std::unique_ptr<Keypad> _keypad;
  std::deque<AccessEvent> _events;

  String _pinBuffer;
  String _lastMessage = "READY";
  uint8_t _failedAttempts = 0;
  unsigned long _lockoutUntilMs = 0;
  bool _lockoutWasActive = false;
  bool _unlockRequested = false;

  void pushEvent(const AccessEvent& event);
};
