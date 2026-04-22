#pragma once

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class OtaCoordinator {
 public:
  // Ada tiga pilihan: diam, pembaruan lewat kabel, atau pembaruan lewat halaman.
  // Nilai ini dipakai supaya dua jenis pembaruan tidak saling tabrakan.
  enum class Mode : uint8_t { Idle, Arduino, Web };

  struct Snapshot {
    // Potret singkat untuk menampilkan keadaan pembaruan.
    // Bagian lain cukup membaca snapshot ini tanpa menyentuh data asli.
    Mode mode = Mode::Idle;
    bool busy = false;
    uint8_t progress = 0;
    String message = "Siap";
  };

  static OtaCoordinator& instance();

  // Siapkan pengunci dan keadaan awal sekali saja.
  // Fungsi ini aman dipanggil lebih dari sekali.
  void begin() const;

  // Mulai pembaruan lewat Arduino IDE / kabel.
  bool beginArduino();
  // Perbarui persentase saat file firmware sedang dikirim.
  void updateArduinoProgress(unsigned int progress, unsigned int total);
  // Tutup mode Arduino OTA dan simpan hasilnya.
  void finishArduino(bool success, const char* message = nullptr);

  // Mulai pembaruan lewat halaman web.
  bool beginWeb(size_t totalBytes, const String& filename);
  // Perbarui persentase saat file dari browser sedang diterima.
  void updateWebProgress(size_t progressBytes, size_t totalBytes);
  // Tutup mode web OTA dan simpan hasilnya.
  void finishWeb(bool success, const String& message);

  // Cek apakah Arduino OTA boleh dijalankan sekarang.
  [[nodiscard]] bool canServeArduino() const;
  // Cek apakah OTA web sedang aktif.
  [[nodiscard]] bool isWebActive() const;
  // Ambil potret keadaan pembaruan saat ini.
  [[nodiscard]] Snapshot snapshot() const;
  // Ubah mode menjadi teks sederhana.
  [[nodiscard]] String modeName() const;

 private:
  OtaCoordinator() = default;

  // Pengunci supaya status pembaruan tidak kacau.
  // Ini mencegah dua tugas menulis data yang sama.
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
