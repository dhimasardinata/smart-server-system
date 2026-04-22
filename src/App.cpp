#include "App.h"

#include "I2CBus.h"
#include "OtaCoordinator.h"
#include "PinMap.h"

#include <Arduino.h>

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

namespace {
// Nama yang dipakai agar perangkat bisa dicari lewat nama jaringan.
constexpr const char* MDNS_HOSTNAME = "monitor-server";
// Ukuran tumpukan kerja untuk tugas latar belakang.
constexpr uint32_t BACKGROUND_TASK_STACK_WORDS = 6144;
// Prioritas tugas latar belakang.
constexpr UBaseType_t BACKGROUND_TASK_PRIORITY = 1;
// Tugas latar belakang berjalan di core 0.
constexpr BaseType_t BACKGROUND_TASK_CORE = 0;
// Seberapa sering tugas latar belakang berjalan.
constexpr TickType_t BACKGROUND_TASK_PERIOD_TICKS = pdMS_TO_TICKS(10);
// Ukuran tumpukan kerja untuk tugas pembaruan.
constexpr uint32_t OTA_TASK_STACK_WORDS = 4096;
// Prioritas tugas pembaruan.
constexpr UBaseType_t OTA_TASK_PRIORITY = 2;
// Tugas pembaruan juga dijalankan di core 0.
constexpr BaseType_t OTA_TASK_CORE = 0;
// Seberapa sering tugas pembaruan dijalankan.
constexpr TickType_t OTA_TASK_PERIOD_TICKS = pdMS_TO_TICKS(5);

uint32_t relayInactiveLevel(bool activeLow) {
  // Tentukan level pin saat alat harus mati.
  return activeLow ? 1U : 0U;
}

uint32_t relayOutputLevel(bool activeLow, bool on) {
  // Tentukan level pin saat alat harus nyala atau mati.
  return activeLow ? (on ? 0U : 1U) : (on ? 1U : 0U);
}

bool relayActiveLow(uint8_t pin) {
  // Beberapa saklar di papan ini punya aturan nyala yang berbeda.
  if (pin == Pins::RELAY_SOLENOID)
    return Pins::SOLENOID_ACTIVE_LOW;
  if (pin == Pins::RELAY_ALERT)
    return Pins::ALERT_ACTIVE_LOW;
  return Pins::RELAY_ACTIVE_LOW;
}

void prepareRelayPin(uint8_t pin) {
  // Ubah angka pin biasa jadi bentuk yang dipakai driver rendah.
  const gpio_num_t gpio = static_cast<gpio_num_t>(pin);
  // Cari level mati yang cocok untuk pin ini.
  const uint32_t inactiveLevel = relayInactiveLevel(relayActiveLow(pin));
  // Saat baru menyala, semua saklar dipaksa mati dulu.
  gpio_reset_pin(gpio);
  gpio_set_level(gpio, inactiveLevel);
  gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
}
}  // namespace

// Hubungkan layar saat objek App dibuat.
App::App() : _display(Pins::I2C_ADDR_LCD, Pins::LCD_COLS, Pins::LCD_ROWS) {}

void App::setRelay(uint8_t pin, bool on) {
  // Ubah pin angka menjadi bentuk yang dibutuhkan sistem pin rendah.
  const gpio_num_t gpio = static_cast<gpio_num_t>(pin);
  // Cari level sinyal yang sesuai untuk kondisi nyala/mati.
  const uint32_t level = relayOutputLevel(relayActiveLow(pin), on);
  // Simpan penanda agar nilai yang sama tidak ditulis terus-menerus.
  bool* cachedState = nullptr;
  if (pin == Pins::RELAY_FAN1) {
    cachedState = &_fan1RelayApplied;
  } else if (pin == Pins::RELAY_FAN2) {
    cachedState = &_fan2RelayApplied;
  } else if (pin == Pins::RELAY_SOLENOID) {
    cachedState = &_solenoidRelayApplied;
  } else if (pin == Pins::RELAY_ALERT) {
    cachedState = &_alertRelayApplied;
  }

  if (cachedState != nullptr && *cachedState == on)
    return;

  // Hindari menulis nilai yang sama berulang-ulang supaya pin lebih stabil.
  gpio_set_level(gpio, level);
  // Simpan keadaan terbaru jika relay ini memang dipantau.
  if (cachedState != nullptr)
    *cachedState = on;
  if (pin == Pins::RELAY_FAN1 || pin == Pins::RELAY_FAN2 ||
      pin == Pins::RELAY_SOLENOID || pin == Pins::RELAY_ALERT) {
    // Setelah relay berubah, layar perlu diberi kesempatan menyesuaikan.
    _display.scheduleRecovery(pin == Pins::RELAY_SOLENOID ? 250 : 25);
  }
}

void App::setupRelays() {
  // Semua saklar dimulai dari mati lalu catatan status direset.
  prepareRelayPin(Pins::RELAY_FAN1);
  prepareRelayPin(Pins::RELAY_FAN2);
  prepareRelayPin(Pins::RELAY_SOLENOID);
  prepareRelayPin(Pins::RELAY_ALERT);
  _fan1On = false;
  _fan2On = false;
  _solenoidOn = false;
  _solenoidUnlockUntilMs = 0;
  _alertOn = false;
  _fan1RelayApplied = false;
  _fan2RelayApplied = false;
  _solenoidRelayApplied = false;
  _alertRelayApplied = false;
  _alertState = AlertState::Idle;
  _thermalTier = 0;
  _alertUntilMs = 0;
  _alertQueueHead = 0;
  _alertQueueTail = 0;
  _alertQueueCount = 0;
  setRelay(Pins::RELAY_FAN1, false);
  setRelay(Pins::RELAY_FAN2, false);
  setRelay(Pins::RELAY_SOLENOID, false);
  setRelay(Pins::RELAY_ALERT, false);
}

void App::setup() {
  // Urutan awal dibuat hati-hati: saklar dulu, lalu penyimpanan,
  // lalu alat baca, sambungan, dan layar.
  setupRelays();

  // Buka jalur teks ke Serial Monitor.
  Serial.begin(115200);
  // Beri jeda singkat supaya serial siap.
  delay(100);

  // Matikan pengawas tugas bawaan yang tidak dipakai di alur ini.
  esp_task_wdt_deinit();
  // Kurangi suara log bawaan agar pesan penting lebih mudah dibaca.
  esp_log_level_set("task_wdt", ESP_LOG_NONE);
  esp_log_level_set("esp32-hal-ledc", ESP_LOG_WARN);
  esp_log_level_set("Preferences", ESP_LOG_WARN);

  // Siapkan jalur I2C dan pengatur pembaruan.
  I2CBus::begin();
  OtaCoordinator::instance().begin();

  // Baca setelan dari memori internal.
  if (!_config.begin())
    Serial.println(F("Config init failed"));
  // Interval baca sensor mengikuti setelan yang disimpan.
  _sensors.setReadIntervalMs(_config.data.sensorReadIntervalSec * 1000UL);

  // Hidupkan layar kalau tersedia.
  if (_display.begin())
    _display.showStartup();

  // Siapkan sensor dan keypad akses.
  _sensors.begin();
  _access.begin(&_config);

  // Siapkan WiFi dan layanan jaringan.
  _wifi.begin(&_config);
  _network.begin(&_config, &_wifi, &_sensors, &_access);

  // Kalau jaringan sudah hidup, siapkan pembaruan lewat jaringan.
  if (_wifi.isConnected())
    setupOTA();

  // Ambil data awal agar layar tidak kosong.
  const SensorData initialData = _sensors.getData();
  // Simpan salinan awal untuk tugas latar belakang.
  publishRuntimeSnapshot(initialData);
  // Tampilkan data awal ke layar.
  updateDisplay(initialData);
  // Tampilkan menu yang sesuai keadaan awal.
  renderCurrentUiState();
  // Hidupkan tugas yang menangani jaringan dan data.
  startBackgroundTask();
  // Hidupkan tugas pembaruan jika perlu.
  startOtaTask();
}

void App::setupOTA() {
  if (_otaReady)
    return;

  // Nama ini memudahkan perangkat ditemukan tanpa menghafal alamatnya.
  // Ini dipakai oleh alat pembaruan bawaan ESP32.
  if (MDNS.begin(MDNS_HOSTNAME)) {
    Serial.printf("mDNS: %s.local\n", MDNS_HOSTNAME);
  } else {
    Serial.printf("mDNS failed for %s.local\n", MDNS_HOSTNAME);
  }

  ArduinoOTA.setHostname(MDNS_HOSTNAME);
  ArduinoOTA.setPort(3232);
  ArduinoOTA.onStart([]() {
    // Tolak kalau web OTA sedang aktif.
    if (!OtaCoordinator::instance().beginArduino()) {
      Serial.println(F("OTA start rejected: web OTA active"));
      return;
    }
    // Beri tanda bahwa pembaruan kabel sudah mulai.
    Serial.println(F("OTA start"));
  });
  ArduinoOTA.onEnd([]() {
    // Pembaruan lewat kabel selesai.
    Serial.println(F("OTA done"));
    OtaCoordinator::instance().finishArduino(true);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Simpan kemajuan supaya bisa ditampilkan.
    OtaCoordinator::instance().updateArduinoProgress(progress, total);
    // Tulis persen kemajuan ke Serial Monitor.
    Serial.printf("OTA %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    // Kalau gagal, tulis kode error lalu tutup mode ini.
    Serial.printf("OTA error[%u]\n", error);
    OtaCoordinator::instance().finishArduino(false);
  });
  ArduinoOTA.begin();
  Serial.printf("OTA ready: %s.local (%s:3232)\n", MDNS_HOSTNAME,
                WiFi.localIP().toString().c_str());
  _otaReady = true;
}

void App::updateThermalAndFans(const SensorData& data) {
  // Aturan kipas:
  // - kipas 1 ikut aktif kalau memang perlu
  // - kipas 2 baru aktif saat kondisi makin panas/lembap
  // Peringatan aktif kalau data valid dan melewati batas.
  // Jadi dua kipas tidak selalu hidup bersamaan.
  // Logika ini menjaga suhu tetap aman tanpa boros tenaga.
  _warning = data.valid && ((data.temperature > _config.data.warnThresholdC) || (data.humidity > _config.data.warnHumPct));
  // Kipas kedua menyala saat kondisi sudah lebih tinggi lagi.
  _fan2On = data.valid && ((data.temperature >= _config.data.stage2ThresholdC) || (data.humidity >= _config.data.stage2HumPct));
  // Kipas pertama mengikuti aturan dasar atau saat ada peringatan.
  _fan1On = _config.data.fan1BaselineOn || _warning || _fan2On;

  // Terapkan keadaan kipas ke relay.
  setRelay(Pins::RELAY_FAN1, _fan1On);
  setRelay(Pins::RELAY_FAN2, _fan2On);
  // Ubah status tanda bahaya sesuai kondisi termal.
  updateThermalAlertState(_warning, _fan2On);
}

void App::requestUnlock() {
  if (_solenoidOn && _solenoidUnlockUntilMs > millis()) {
    // Kalau pintu masih dalam masa buka, jangan buka lagi.
    Serial.println(F("Solenoid unlock request ignored while already active"));
    return;
  }

  // Kunci pintu dibuka sebentar lalu ditutup lagi otomatis.
  // Waktu buka ditentukan dari setelan yang disimpan.
  _solenoidOn = true;
  // Catat kapan pintu harus dikunci lagi.
  _solenoidUnlockUntilMs = millis() + (_config.data.solenoidUnlockSec * 1000UL);
  Serial.printf("Solenoid unlock for %lu seconds\n",
                static_cast<unsigned long>(_config.data.solenoidUnlockSec));
  // Nyalakan relay untuk membuka solenoid.
  setRelay(Pins::RELAY_SOLENOID, true);
}

void App::startUnlockSession(const String& displayName) {
  // Minta pintu dibuka.
  requestUnlock();
  // Kalau PIN benar, minta pintu dibuka lalu hapus tanda permintaannya.
  // Ini mencegah permintaan lama diproses dua kali.
  _access.consumeUnlockRequest();
  // Pindah ke tampilan sukses.
  _uiState = UIState::UNLOCK_OK;
  // Simpan waktu mulai tampilan sukses.
  _unlockOkMs = millis();
  // Tampilkan nama pengguna di layar.
  _display.showUnlockOk(displayName);
}

void App::updateSolenoid() {
  // Kalau waktu buka habis, kunci pintu lagi.
  // Tujuannya supaya pintu tidak terbuka terus.
  if (_solenoidOn && _solenoidUnlockUntilMs > 0 &&
      millis() >= _solenoidUnlockUntilMs) {
    _solenoidOn = false;
    _solenoidUnlockUntilMs = 0;
    Serial.println(F("Solenoid locked"));
    // Matikan relay agar pintu terkunci lagi.
    setRelay(Pins::RELAY_SOLENOID, false);
  }
}

void App::setAlertRelay(bool on) {
  // Saklar alarm dipakai untuk tanda bahaya atau akses ditolak.
  _alertOn = on;
  // Terapkan status alarm ke relay.
  setRelay(Pins::RELAY_ALERT, on);
}

bool App::hasContinuousThermalAlert() const {
  return _thermalTier > 0;
}

uint16_t App::alertDurationMs(AlertState state) const {
  switch (state) {
    case AlertState::AccessDenied:
      return 2000;
    default:
      return 0;
  }
}

const char* App::alertStateName() const {
  return alertStateName(_alertState);
}

const char* App::alertStateName(AlertState state) {
  switch (state) {
    case AlertState::AccessGranted:
      return "ACCESS_GRANTED";
    case AlertState::AccessDenied:
      return "ACCESS_DENIED";
    case AlertState::Lockout:
      return "LOCKOUT";
    case AlertState::ThermalWarning:
      return "THERMAL_WARNING";
    case AlertState::ThermalCritical:
      return "THERMAL_CRITICAL";
    case AlertState::Idle:
    default:
      return "IDLE";
  }
}

void App::startBackgroundTask() {
  // Bagian belakang memisahkan urusan sambungan dari putaran utama.
  // Buat pengunci agar data tidak ditulis bersamaan.
  // Kalau ini gagal, program tetap jalan dengan satu loop utama.
  _runtimeMutex = xSemaphoreCreateMutex();
  if (_runtimeMutex == nullptr) {
    Serial.println(
        F("App: runtime mutex init failed, keeping single-loop mode"));
    return;
  }

  if (xTaskCreatePinnedToCore(App::backgroundTaskEntry, "app_background",
                              BACKGROUND_TASK_STACK_WORDS, this,
                              BACKGROUND_TASK_PRIORITY, &_backgroundTask,
                              BACKGROUND_TASK_CORE) != pdPASS) {
    Serial.println(
        F("App: background task init failed, keeping single-loop mode"));
    return;
  }

  _backgroundTaskEnabled = true;
}

void App::startOtaTask() {
  // Bagian pembaruan dipisah supaya tidak mengganggu pekerjaan lain.
  // Ini mencegah proses upload firmware menahan tampilan utama.
  if (xTaskCreatePinnedToCore(App::otaTaskEntry, "app_ota",
                              OTA_TASK_STACK_WORDS, this, OTA_TASK_PRIORITY,
                              &_otaTask, OTA_TASK_CORE) != pdPASS) {
    Serial.println(F("App: OTA task init failed, keeping loop OTA mode"));
    return;
  }

  _otaTaskEnabled = true;
}

void App::publishRuntimeSnapshot(const SensorData& data) {
  // Salinan singkat ini dipakai bagian lain tanpa menyentuh data utama.
  // Jadi tampilan dan jaringan membaca data yang sama.
  // Data utama tetap aman meski ada tugas lain berjalan.
  if (_runtimeMutex != nullptr &&
      xSemaphoreTake(_runtimeMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    // Salin semua keadaan terbaru ke snapshot bersama.
    _runtimeSnapshot.data = data;
    _runtimeSnapshot.fan1On = _fan1On;
    _runtimeSnapshot.fan2On = _fan2On;
    _runtimeSnapshot.warning = _warning;
    _runtimeSnapshot.solenoidOn = _solenoidOn;
    _runtimeSnapshot.alertOn = _alertOn;
    _runtimeSnapshot.alertState = _alertState;
    xSemaphoreGive(_runtimeMutex);
    return;
  }

  _runtimeSnapshot.data = data;
  _runtimeSnapshot.fan1On = _fan1On;
  _runtimeSnapshot.fan2On = _fan2On;
  _runtimeSnapshot.warning = _warning;
  _runtimeSnapshot.solenoidOn = _solenoidOn;
  _runtimeSnapshot.alertOn = _alertOn;
  _runtimeSnapshot.alertState = _alertState;
}

App::RuntimeSnapshot App::readRuntimeSnapshot() {
  // Ambil salinan snapshot agar aman dipakai di tugas lain.
  RuntimeSnapshot snapshot = _runtimeSnapshot;
  if (_runtimeMutex != nullptr &&
      xSemaphoreTake(_runtimeMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    snapshot = _runtimeSnapshot;
    xSemaphoreGive(_runtimeMutex);
  }
  return snapshot;
}

void App::backgroundTaskEntry(void* context) {
  static_cast<App*>(context)->backgroundTaskLoop();
}

void App::otaTaskEntry(void* context) {
  static_cast<App*>(context)->otaTaskLoop();
}

void App::backgroundTaskLoop() {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    // Selalu cek sambungan lalu kirim ringkasan data terbaru.
    // Bagian ini berjalan terus selama alat menyala.
    // Perbarui status WiFi.
    _wifi.update();

    // Baca snapshot terbaru yang sudah disiapkan.
    const RuntimeSnapshot snapshot = readRuntimeSnapshot();
    // Kirim data ke layanan jaringan.
    _network.update(snapshot.data, snapshot.fan1On, snapshot.fan2On,
                    snapshot.warning, snapshot.solenoidOn, snapshot.alertOn,
                    alertStateName(snapshot.alertState));

    vTaskDelayUntil(&lastWake, BACKGROUND_TASK_PERIOD_TICKS);
  }
}

void App::otaTaskLoop() {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    // Kalau sambungan aktif dan pembaruan siap, baru layani unggah program.
    // Kalau tidak, mode ini tidak perlu membuat jaringan terbuka.
    if (_wifi.isConnected() && OtaCoordinator::instance().canServeArduino()) {
      // Pastikan mode OTA sudah siap.
      setupOTA();
      // Layani permintaan pembaruan yang masuk.
      ArduinoOTA.handle();
    } else if (_otaReady) {
      // Kalau tidak boleh dipakai, tutup nama jaringan sementara.
      MDNS.end();
      _otaReady = false;
    }

    vTaskDelayUntil(&lastWake, OTA_TASK_PERIOD_TICKS);
  }
}

void App::enqueueAlert(AlertState state) {
  if (state == AlertState::Idle || hasContinuousThermalAlert())
    return;

  // Daftar alarm dipakai supaya tanda peringatan muncul berurutan.
  // Kalau penuh, alarm paling lama digeser keluar.
  // Dengan begini, pesan baru tetap sempat terlihat.
  if (_alertQueueCount >= ALERT_QUEUE_SIZE) {
    _alertQueueHead = (_alertQueueHead + 1) % ALERT_QUEUE_SIZE;
    --_alertQueueCount;
  }

  _alertQueue[_alertQueueTail] = state;
  _alertQueueTail = (_alertQueueTail + 1) % ALERT_QUEUE_SIZE;
  ++_alertQueueCount;
}

void App::clearQueuedAlerts(bool stopCurrent) {
  _alertQueueHead = 0;
  _alertQueueTail = 0;
  _alertQueueCount = 0;

  if (stopCurrent)
    stopAlert();
}

bool App::dequeueAlert(AlertState& state) {
  if (_alertQueueCount == 0)
    return false;
  state = _alertQueue[_alertQueueHead];
  _alertQueueHead = (_alertQueueHead + 1) % ALERT_QUEUE_SIZE;
  --_alertQueueCount;
  return true;
}

void App::startAlert(AlertState state) {
  if (hasContinuousThermalAlert())
    return;
  const uint16_t durationMs = alertDurationMs(state);
  if (durationMs == 0)
    return;

  _alertState = state;
  _alertUntilMs = millis() + durationMs;
  setAlertRelay(true);
}

void App::stopAlert() {
  _alertUntilMs = 0;
  _alertState = AlertState::Idle;
  setAlertRelay(false);
}

void App::updateThermalAlertState(bool warning, bool critical) {
  // Jika suhu atau lembap melewati batas, tanda bahaya ini jadi utama.
  // Ubah tingkat bahaya berdasarkan kondisi terbaru.
  // Level lebih tinggi berarti keadaan lebih berbahaya.
  // Kalau level turun, alarm juga ikut reda.
  const uint8_t newTier = critical ? 2 : (warning ? 1 : 0);
  if (newTier == _thermalTier)
    return;

  _thermalTier = newTier;
  if (_thermalTier > 0) {
    _alertQueueHead = 0;
    _alertQueueTail = 0;
    _alertQueueCount = 0;
    _alertUntilMs = 0;
    _alertState = _thermalTier >= 2 ? AlertState::ThermalCritical
                                    : AlertState::ThermalWarning;
    setAlertRelay(true);
    return;
  }

  stopAlert();
}

void App::handleAccessAlert(const AccessEvent& event) {
  if (hasContinuousThermalAlert())
    return;

  // Akses sukses menghentikan tanda lama yang belum sempat muncul.
  // Akses gagal justru menambah tanda peringatan baru.
  // Tujuannya supaya layar memberi tahu keadaan yang paling baru.
  switch (event.type) {
    case AccessEventType::AccessGranted:
      clearQueuedAlerts(true);
      break;
    case AccessEventType::AccessDenied:
      clearQueuedAlerts(true);
      enqueueAlert(AlertState::AccessDenied);
      break;
    case AccessEventType::LockoutStarted:
      clearQueuedAlerts(true);
    case AccessEventType::LockoutEnded:
    default:
      break;
  }
}

void App::updateAlert() {
  if (hasContinuousThermalAlert()) {
    // Tanda bahaya tetap aktif selama kondisi buruk masih ada.
    // Selama suhu masih tinggi, alarm tidak boleh padam.
    _alertState = _thermalTier >= 2 ? AlertState::ThermalCritical
                                    : AlertState::ThermalWarning;
    if (!_alertOn)
      setAlertRelay(true);
    return;
  }

  if (_alertState == AlertState::ThermalWarning ||
      _alertState == AlertState::ThermalCritical) {
    _alertState = AlertState::Idle;
  }

  if (_alertState == AlertState::Idle) {
    AlertState nextState;
    if (dequeueAlert(nextState)) {
      startAlert(nextState);
    } else if (_alertOn) {
      setAlertRelay(false);
    }
    return;
  }

  if (millis() >= _alertUntilMs)
    stopAlert();
}

void App::updateDisplay(const SensorData& data) {
  // Salin informasi WiFi agar layar bisa menampilkannya.
  _display.setWifiInfo(_wifi.isConnected() || _wifi.isApMode(),
                       _wifi.getIP().toString());
  // Salin data sensor dan status kipas ke layar.
  _display.setTelemetry(data.temperature, data.humidity, data.valid, _fan1On,
                        _fan2On, _warning);
  // Salin status keamanan ke layar.
  _display.setSecurity(_solenoidOn ? "UNLOCKING" : "LOCKED",
                       _access.lastMessage(), _access.isLockoutActive(),
                       _access.lockoutRemainingSec());
}

void App::renderCurrentUiState() {
  // Tampilan dipilih sesuai keadaan terakhir di layar.
  // Fungsi ini hanya menggambar ulang, bukan mengubah data.
  switch (_uiState) {
    case UIState::MONITORING:
      if (_wifi.isApMode()) {
        // Kalau sedang AP mode, tampilkan alamat akses sementara.
        const String apIp = _wifi.getIP().toString();
        _display.showApMode(std::string_view(apIp.c_str(), apIp.length()));
      } else {
        // Kalau tidak, tampilkan layar utama.
        _display.showMainScreen();
      }
      break;

    case UIState::PIN_ENTRY:
      if (_access.isLockoutActive()) {
        _display.showPinEntry(0, true, _access.lockoutRemainingSec());
      } else {
        _display.showPinEntry(_pinBuf.length());
      }
      break;

    case UIState::UNLOCK_OK:
      _display.showUnlockOk(_authDisplayName);
      break;

    case UIState::ADMIN_MENU:
      _display.showAdminMenu();
      break;

    case UIState::USER_LIST:
      // Tampilkan daftar pengguna yang ada.
      _display.showUserList(_config.data.users.data(), MAX_USERS,
                            _userListAction);
      break;

    case UIState::CHANGE_PIN:
      _display.showChangePin(
          _selectedUserId, _changePinStep,
          _changePinStep == 0 ? _pinBuf.length() : _confirmBuf.length());
      break;

    case UIState::ADD_USER:
      _display.showAddUser(_autoUserId, _pinBuf.length());
      break;

    case UIState::CONFIRM_DELETE:
      _display.showConfirmDelete(_selectedUserId);
      break;

    case UIState::STATUS_MESSAGE:
      // Pesan sementara ini ditutup oleh timer, jadi tidak perlu gambar baru.
      break;
  }
}

void App::showTransientMessage(const char* title, const char* msg, bool success,
                               UIState returnState, unsigned long durationMs) {
  // Pindah sementara ke layar pesan.
  _uiState = UIState::STATUS_MESSAGE;
  // Simpan keadaan yang akan dituju setelah pesan hilang.
  _statusReturnState = returnState;
  // Catat kapan pesan harus ditutup.
  _statusUntilMs = millis() + durationMs;
  // Tulis pesan ke layar.
  _display.showMessage(title, msg, success);
}

void App::resetToMonitoring() {
  // Balik ke layar utama.
  _uiState = UIState::MONITORING;
  // Hapus batas waktu pesan sementara.
  _statusUntilMs = 0;
  // Kosongkan PIN sementara.
  _pinBuf = "";
  // Kosongkan konfirmasi PIN.
  _confirmBuf = "";
  // Balikkan langkah ganti PIN.
  _changePinStep = 0;
  // Tampilkan layar sesuai keadaan baru.
  renderCurrentUiState();
}

void App::buildUserSlotMap() {
  // Kosongkan daftar slot dulu.
  _userSlotCount = 0;
  for (size_t i = 0; i < MAX_USERS; ++i) {
    // Lewati slot kosong atau pengguna yang tidak aktif.
    if (_config.data.users[i].userId.length() == 0 ||
        !_config.data.users[i].enabled)
      continue;
    // Saat hapus user, admin pertama tidak ikut dipilih.
    if (_userListAction == 1 && i == 0)
      continue;
    if (_userSlotCount < MAX_SLOTS) {
      // Simpan indeks pengguna yang bisa dipilih.
      _userSlotMap[_userSlotCount++] = static_cast<uint8_t>(i);
    }
  }
}

void App::popLastDigit(String& buffer) {
  if (buffer.length() == 0)
    return;
  // Hapus satu digit terakhir dari buffer.
  buffer.remove(buffer.length() - 1);
}

void App::handleUIKey(char key) {
  if (key == NO_KEY)
    return;

  // Catat waktu terakhir pengguna menekan tombol.
  // Ini dipakai untuk tahu kapan menu terlalu lama diam.
  _uiIdleMs = millis();

  switch (_uiState) {
    case UIState::MONITORING: {
      if (key == 'A') {
        // Tombol A membuka layar input PIN.
        _uiState = UIState::PIN_ENTRY;
        // Kosongkan buffer PIN lama.
        _pinBuf = "";
        if (_access.isLockoutActive()) {
          // Kalau terkunci, tampilkan hitung mundur.
          _display.showPinEntry(0, true, _access.lockoutRemainingSec());
        } else {
          // Kalau tidak, tampilkan layar kosong.
          _display.showPinEntry(0);
        }
      }
      break;
    }

    case UIState::PIN_ENTRY: {
      if (_access.isLockoutActive()) {
        // Saat terkunci, hanya boleh melihat sisa waktu.
        // Input PIN baru tidak diterima sampai waktu habis.
        _display.showPinEntry(0, true, _access.lockoutRemainingSec());
        if (key == '*')
          // Tombol bintang bisa kembali ke tampilan utama.
          resetToMonitoring();
        break;
      }
      if (key >= '0' && key <= '9' && _pinBuf.length() < 8) {
        // Tambah digit PIN.
        _pinBuf += key;
        _display.showPinEntry(_pinBuf.length());
      } else if (key == 'D') {
        // Tombol D dipakai untuk menghapus digit terakhir.
        popLastDigit(_pinBuf);
        _display.showPinEntry(_pinBuf.length());
      } else if (key == '*') {
        // Tombol bintang membatalkan input.
        resetToMonitoring();
      } else if (key == '#' && _pinBuf.length() >= 4) {
        // Tombol pagar dipakai untuk mengirim PIN.
        AuthResult auth = _access.validatePin(_pinBuf);
        _pinBuf = "";
        if (auth.success) {
          // Simpan identitas pengguna yang berhasil masuk.
          _authUserId = auth.userId;
          _authDisplayName = auth.displayName;
          if (auth.isAdmin) {
            // Admin masuk ke menu khusus.
            _uiState = UIState::ADMIN_MENU;
            _display.showAdminMenu();
          } else {
            // Pengguna biasa hanya membuka pintu sebentar.
            startUnlockSession(auth.displayName);
          }
        } else {
          if (_access.isLockoutActive()) {
            // Kalau terkunci, tetap tampilkan hitung mundur.
            _display.showPinEntry(0, true, _access.lockoutRemainingSec());
          } else {
            // Kalau salah, tampilkan pesan gagal.
            _pinBuf = "";
            showTransientMessage("PIN SALAH", "Coba lagi", false,
                                 UIState::PIN_ENTRY, 1500);
          }
        }
      }
      break;
    }

    case UIState::UNLOCK_OK: {
      resetToMonitoring();
      break;
    }

    case UIState::ADMIN_MENU: {
      if (key == '*') {
        // Keluar dari menu admin.
        resetToMonitoring();
      } else if (key == '1') {
        // Menu 1: buka pintu.
        startUnlockSession(_authDisplayName);
      } else if (key == '2') {
        // Menu 2: lihat daftar pengguna.
        _userListAction = 0;
        buildUserSlotMap();
        _uiState = UIState::USER_LIST;
        _display.showUserList(_config.data.users.data(), MAX_USERS, 0);
      } else if (key == '3') {
        // Menu 3: tambah pengguna baru.
        _autoUserId = _access.generateUserId();
        _pinBuf = "";
        _uiState = UIState::ADD_USER;
        _display.showAddUser(_autoUserId, 0);
      } else if (key == '4') {
        // Menu 4: hapus pengguna.
        _userListAction = 1;
        buildUserSlotMap();
        _uiState = UIState::USER_LIST;
        _display.showUserList(_config.data.users.data(), MAX_USERS, 1);
      }
      break;
    }

    case UIState::USER_LIST: {
      if (key == '*') {
        // Balik ke menu admin kalau pengguna membatalkan.
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '1' && key <= '9') {
        // Pilih salah satu nama dari daftar.
        uint8_t slot = key - '1';
        if (slot < _userSlotCount) {
          uint8_t idx = _userSlotMap[slot];
          _selectedUserId = _config.data.users[idx].userId;
          if (_userListAction == 0) {
            _uiState = UIState::CHANGE_PIN;
            _changePinStep = 0;
            _pinBuf = "";
            _confirmBuf = "";
            _display.showChangePin(_selectedUserId, 0, 0);
          } else {
            _uiState = UIState::CONFIRM_DELETE;
            _display.showConfirmDelete(_selectedUserId);
          }
        }
      }
      break;
    }

    case UIState::CHANGE_PIN: {
      if (key == '*') {
        // Batal dan kembali ke menu admin.
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '0' && key <= '9') {
        if (_changePinStep == 0 && _pinBuf.length() < 8) {
          // Isi PIN baru.
          _pinBuf += key;
          _display.showChangePin(_selectedUserId, 0, _pinBuf.length());
        } else if (_changePinStep == 1 && _confirmBuf.length() < 8) {
          // Isi ulang PIN untuk konfirmasi.
          _confirmBuf += key;
          _display.showChangePin(_selectedUserId, 1, _confirmBuf.length());
        }
      } else if (key == 'D') {
        if (_changePinStep == 0) {
          // Hapus digit PIN baru.
          popLastDigit(_pinBuf);
          _display.showChangePin(_selectedUserId, 0, _pinBuf.length());
        } else {
          // Hapus digit PIN konfirmasi.
          popLastDigit(_confirmBuf);
          _display.showChangePin(_selectedUserId, 1, _confirmBuf.length());
        }
      } else if (key == '#') {
        if (_changePinStep == 0 && _pinBuf.length() >= 4) {
          // Lanjut ke langkah konfirmasi.
          _changePinStep = 1;
          _confirmBuf = "";
          _display.showChangePin(_selectedUserId, 1, 0);
        } else if (_changePinStep == 1 && _confirmBuf.length() >= 4) {
          // Kalau dua PIN cocok, simpan perubahan.
          if (_pinBuf == _confirmBuf) {
            String error;
            if (_access.changePin(_selectedUserId, _pinBuf, error)) {
              // Tampilkan pesan berhasil.
              showTransientMessage("BERHASIL", "PIN diperbarui", true,
                                   UIState::ADMIN_MENU, 2000);
            } else {
              // Kalau gagal simpan, tampilkan alasan.
              showTransientMessage("GAGAL", error.c_str(), false,
                                   UIState::ADMIN_MENU, 2000);
            }
          } else {
            // Kalau tidak cocok, beri tahu pengguna.
            showTransientMessage("GAGAL", "PIN tidak cocok", false,
                                 UIState::ADMIN_MENU, 2000);
          }
        }
      }
      break;
    }

    case UIState::ADD_USER: {
      if (key == '*') {
        // Batal dan kembali ke menu admin.
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key >= '0' && key <= '9' && _pinBuf.length() < 8) {
        // Isi PIN untuk pengguna baru.
        _pinBuf += key;
        _display.showAddUser(_autoUserId, _pinBuf.length());
      } else if (key == 'D') {
        // Hapus digit terakhir saat salah tekan.
        popLastDigit(_pinBuf);
        _display.showAddUser(_autoUserId, _pinBuf.length());
      } else if (key == '#' && _pinBuf.length() >= 4) {
        // Simpan pengguna baru jika PIN sudah cukup.
        String error;
        if (_access.upsertUser(_autoUserId, _autoUserId, _pinBuf, true,
                               error)) {
          // Pengguna baru berhasil disimpan.
          showTransientMessage("BERHASIL", "User ditambahkan", true,
                               UIState::ADMIN_MENU, 2000);
        } else {
          // Kalau gagal, tampilkan alasan.
          showTransientMessage("GAGAL", error.c_str(), false,
                               UIState::ADMIN_MENU, 2000);
        }
      }
      break;
    }

    case UIState::CONFIRM_DELETE: {
      if (key == '*') {
        // Batal hapus.
        _uiState = UIState::ADMIN_MENU;
        _display.showAdminMenu();
      } else if (key == '1') {
        // Tombol 1 menjadi tanda setuju hapus.
        if (_config.removeUser(_selectedUserId)) {
          // Pengguna berhasil dihapus.
          showTransientMessage("BERHASIL", "User dihapus", true,
                               UIState::ADMIN_MENU, 2000);
        } else {
          // Kalau gagal, beri tahu pengguna.
          showTransientMessage("GAGAL", "Gagal menghapus", false,
                               UIState::ADMIN_MENU, 2000);
        }
      }
      break;
    }

    case UIState::STATUS_MESSAGE:
      break;
  }
}

void App::loop() {
  // Baca sensor secara berkala.
  // Ini jadi sumber data utama untuk kipas dan layar.
  _sensors.update();
  // Perbarui status akses.
  _access.update();

  if (_uiState == UIState::STATUS_MESSAGE && _statusUntilMs > 0 &&
      millis() >= _statusUntilMs) {
    // Kalau pesan sementara habis waktunya, balik ke layar tujuan.
    // Dengan begitu, layar tidak terjebak di pesan lama.
    _uiState = _statusReturnState;
    _statusUntilMs = 0;
    renderCurrentUiState();
  }

  // Ambil tombol yang sedang ditekan.
  const char key = _access.getKey();
  if (_uiState != UIState::STATUS_MESSAGE) {
    // Tombol hanya diproses kalau bukan layar pesan sementara.
    // Pesan sementara tidak boleh diganggu tombol lain.
    handleUIKey(key);
  }

  // Kalau ada permintaan buka pintu, jalankan.
  if (_access.consumeUnlockRequest() && _uiState != UIState::UNLOCK_OK) {
    // Kalau ada permintaan buka pintu, jalankan sekali saja.
    requestUnlock();
  }

  // Ambil data sensor terbaru.
  const SensorData data = _sensors.getData();
  // Terapkan aturan kipas.
  updateThermalAndFans(data);
  // Cek apakah waktu buka pintu sudah selesai.
  updateSolenoid();

  AccessEvent event;
  // Proses semua kejadian akses yang menunggu.
  // Semua catatan itu diteruskan ke layar dan jaringan.
  while (_access.popEvent(event)) {
    handleAccessAlert(event);
    _network.logAccessEvent(event);
  }

  // Perbarui tanda bahaya.
  updateAlert();
  // Tampilkan data terbaru di layar.
  updateDisplay(data);
  // Cek apakah layar perlu dipulihkan.
  _display.maintainConnection();
  if (_display.consumeRecovered()) {
    // Kalau layar pulih, tampilkan ulang tampilan aktif.
    renderCurrentUiState();
  }
  // Simpan snapshot data untuk bagian lain.
  publishRuntimeSnapshot(data);

  if (!_otaTaskEnabled) {
    if (_wifi.isConnected() && OtaCoordinator::instance().canServeArduino()) {
      // Kalau tugas OTA belum aktif, layani secara langsung.
      setupOTA();
      ArduinoOTA.handle();
    } else if (_otaReady) {
      // Kalau tidak dipakai, tutup mDNS.
      MDNS.end();
      _otaReady = false;
    }
  }

  if (!_backgroundTaskEnabled) {
    // Kalau belum ada tugas latar belakang, jalankan manual.
    // Ini jadi jalur cadangan kalau tugas terpisah gagal dibuat.
    _wifi.update();
    _network.update(data, _fan1On, _fan2On, _warning, _solenoidOn, _alertOn,
                    alertStateName());
  }

  if (_uiState == UIState::MONITORING) {
    if (_wifi.isApMode()) {
      // Saat AP mode, refresh layar secara berkala.
      // Tujuannya supaya alamat setup tetap terlihat.
      if (millis() - _lastApDisplayRefreshMs >= AP_DISPLAY_REFRESH_MS) {
        _lastApDisplayRefreshMs = millis();
        renderCurrentUiState();
      }
    } else {
      // Kalau normal, biarkan layar menangani scrolling.
      _display.loop();
    }
  }

  if (_uiState == UIState::PIN_ENTRY && _access.isLockoutActive() &&
      millis() - _lastLockoutRefreshMs >= LOCKOUT_REFRESH_MS) {
    // Saat terkunci, perbarui hitung mundur secara berkala.
    // Agar pengguna tahu sisa waktu yang tersisa.
    _lastLockoutRefreshMs = millis();
    renderCurrentUiState();
  }

  if (_uiState == UIState::STATUS_MESSAGE) {
    // Beri jeda kecil saat pesan singkat tampil.
    delay(1);
    return;
  }

  if (_uiState == UIState::UNLOCK_OK &&
      millis() - _unlockOkMs > UNLOCK_DISPLAY_MS) {
    // Setelah pesan sukses selesai, balik ke monitor.
    resetToMonitoring();
  }

  if (_uiState != UIState::MONITORING && _uiState != UIState::UNLOCK_OK &&
      millis() - _uiIdleMs > UI_TIMEOUT_MS) {
    // Kalau terlalu lama diam di menu, kembali ke tampilan utama.
    resetToMonitoring();
  }

  // Kasih kesempatan ke sistem lain untuk jalan.
  delay(0);
}
