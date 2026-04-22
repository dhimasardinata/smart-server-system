#pragma once

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class OtaCoordinator {
 public:
  // Ada tiga pilihan: diam, pembaruan lewat kabel, atau pembaruan lewat halaman.
  enum class Mode : uint8_t { Idle, Arduino, Web };

  struct Snapshot {
    // Potret singkat untuk menampilkan keadaan pembaruan.
    Mode mode = Mode::Idle;
    bool busy = false;
    uint8_t progress = 0;
    String message = "Siap";
  };

  static OtaCoordinator& instance();

  // Siapkan pengunci dan keadaan awal sekali saja.
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

  // Pengunci supaya status pembaruan tidak kacau.
  mutable SemaphoreHandle_t _mutex = nullptr;
  // Mode pembaruan yang sedang aktif.
  Mode _mode = Mode::Idle;
  // Penanda apakah sedang sibuk.
  bool _busy = false;
  // Persentase kemajuan.
  uint8_t _progress = 0;
  // Pesan status yang terakhir.
  String _message = "Siap";

  template <typename Fn>
  auto withLock(Fn&& fn) const -> decltype(fn());
};
