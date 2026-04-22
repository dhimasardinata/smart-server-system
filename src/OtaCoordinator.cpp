#include "OtaCoordinator.h"

namespace {
const char* modeLabel(OtaCoordinator::Mode mode) {
  // Nama ini dipakai saat keadaan pembaruan ingin ditampilkan.
  // Hasilnya dibuat sangat pendek supaya mudah dibaca.
  switch (mode) {
    case OtaCoordinator::Mode::Arduino:
      return "arduino";
    case OtaCoordinator::Mode::Web:
      return "web";
    case OtaCoordinator::Mode::Idle:
    default:
      return "idle";
  }
}
}  // namespace

OtaCoordinator& OtaCoordinator::instance() {
  static OtaCoordinator coordinator;
  return coordinator;
}

void OtaCoordinator::begin() const {
  // Pengaman dibuat sekali supaya keadaannya aman dibaca banyak bagian.
  // Kalau mutex sudah ada, tidak perlu dibuat ulang.
  if (_mutex != nullptr) {
    return;
  }
  _mutex = xSemaphoreCreateMutex();
  if (_mutex == nullptr) {
    Serial.println(F("OTA: coordinator mutex init failed"));
  }
}

bool OtaCoordinator::beginArduino() {
  // Dua cara pembaruan ini tidak boleh berjalan bersamaan.
  // Kalau web OTA sedang aktif, kabel OTA harus menunggu.
  begin();
  return withLock([this]() {
    if (_mode == Mode::Web) {
      return false;
    }
    _mode = Mode::Arduino;
    _busy = true;
    _progress = 0;
    _message = "Arduino OTA berjalan";
    return true;
  });
}

void OtaCoordinator::updateArduinoProgress(unsigned int progress,
                                           unsigned int total) {
  begin();
  withLock([this, progress, total]() {
    _mode = Mode::Arduino;
    _busy = true;
    // Persentase dihitung dari nilai yang masuk dari proses upload.
    _progress =
        total > 0 ? static_cast<uint8_t>(min(100U, (progress * 100U) / total))
                  : 0;
    _message = "Arduino OTA berjalan";
    return 0;
  });
}

void OtaCoordinator::finishArduino(bool success, const char* message) {
  begin();
  withLock([this, success, message]() {
    // Setelah selesai, mode kembali diam.
    _mode = Mode::Idle;
    _busy = false;
    _progress = success ? 100 : 0;
    if (message != nullptr && message[0] != '\0') {
      _message = message;
    } else {
      _message = success ? "Arduino OTA selesai" : "Arduino OTA gagal";
    }
    return 0;
  });
}

bool OtaCoordinator::beginWeb(size_t totalBytes, const String& filename) {
  // Pembaruan lewat halaman punya aturan sendiri dan menyimpan nama file.
  // Nama file ditampilkan supaya pengguna tahu apa yang sedang dipasang.
  begin();
  return withLock([this, totalBytes, &filename]() {
    if (_mode == Mode::Arduino || (_mode == Mode::Web && _busy)) {
      return false;
    }
    _mode = Mode::Web;
    _busy = true;
    _progress = 0;
    _message = "Web OTA: " + filename;
    if (totalBytes == 0) {
      _message += " (ukuran tidak diketahui)";
    }
    return true;
  });
}

void OtaCoordinator::updateWebProgress(size_t progressBytes, size_t totalBytes) {
  begin();
  withLock([this, progressBytes, totalBytes]() {
    // Hitung persentase kemajuan file yang sedang diterima.
    // Nilai ini dipakai oleh halaman status OTA.
    _mode = Mode::Web;
    _busy = true;
    _progress = totalBytes > 0
                    ? static_cast<uint8_t>(
                          min<size_t>(100, (progressBytes * 100U) / totalBytes))
                    : 0;
    _message = "Web OTA berjalan";
    return 0;
  });
}

void OtaCoordinator::finishWeb(bool success, const String& message) {
  begin();
  withLock([this, success, &message]() {
    // Hasil akhir disimpan agar halaman dan layar bisa menampilkannya.
    if (success) {
      _mode = Mode::Web;
      _busy = true;
      _progress = 100;
      _message = message.length() > 0 ? message : "Web OTA selesai";
    } else {
      _mode = Mode::Idle;
      _busy = false;
      _message = message.length() > 0 ? message : "Web OTA gagal";
    }
    return 0;
  });
}

bool OtaCoordinator::canServeArduino() const {
  // Kalau web OTA aktif, kabel OTA harus menunggu.
  // Jadi hanya satu jalur yang boleh berjalan.
  return snapshot().mode != Mode::Web;
}

bool OtaCoordinator::isWebActive() const {
  const Snapshot state = snapshot();
  return state.mode == Mode::Web && state.busy;
}

OtaCoordinator::Snapshot OtaCoordinator::snapshot() const {
  begin();
  return withLock([this]() {
    // Salin keadaan terkini ke potret kecil.
    // Snapshot ini aman dipakai tanpa memegang pengunci terlalu lama.
    Snapshot state;
    state.mode = _mode;
    state.busy = _busy;
    state.progress = _progress;
    state.message = _message;
    return state;
  });
}

String OtaCoordinator::modeName() const {
  // Ubah mode menjadi teks supaya mudah ditampilkan.
  // Ini dipakai di layar, web, dan catatan.
  return String(modeLabel(snapshot().mode));
}

template <typename Fn>
auto OtaCoordinator::withLock(Fn&& fn) const -> decltype(fn()) {
  if (_mutex == nullptr) {
    return fn();
  }

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(20)) != pdTRUE) {
    return fn();
  }

  const auto result = fn();
  xSemaphoreGive(_mutex);
  return result;
}
