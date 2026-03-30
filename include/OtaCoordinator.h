#pragma once

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class OtaCoordinator {
 public:
  enum class Mode : uint8_t { Idle, Arduino, Web };

  struct Snapshot {
    Mode mode = Mode::Idle;
    bool busy = false;
    uint8_t progress = 0;
    String message = "Siap";
  };

  static OtaCoordinator& instance();

  void begin() const;

  bool beginArduino();
  void updateArduinoProgress(unsigned int progress, unsigned int total);
  void finishArduino(bool success, const char* message = nullptr);

  bool beginWeb(size_t totalBytes, const String& filename);
  void updateWebProgress(size_t progressBytes, size_t totalBytes);
  void finishWeb(bool success, const String& message);

  [[nodiscard]] bool canServeArduino() const;
  [[nodiscard]] bool isWebActive() const;
  [[nodiscard]] Snapshot snapshot() const;
  [[nodiscard]] String modeName() const;

 private:
  OtaCoordinator() = default;

  mutable SemaphoreHandle_t _mutex = nullptr;
  Mode _mode = Mode::Idle;
  bool _busy = false;
  uint8_t _progress = 0;
  String _message = "Siap";

  template <typename Fn>
  auto withLock(Fn&& fn) const -> decltype(fn());
};
