#pragma once

#include "AccessController.h"
#include "Config.h"
#include "Display.h"
#include "NetworkServices.h"
#include "Sensors.h"
#include "WiFiHandler.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

enum class UIState : uint8_t {
  // Keadaan layar yang bisa muncul.
  // Dipakai supaya program tahu halaman mana yang sedang dilihat.
  MONITORING,
  PIN_ENTRY,
  UNLOCK_OK,
  ADMIN_MENU,
  USER_LIST,
  CHANGE_PIN,
  ADD_USER,
  CONFIRM_DELETE,
  STATUS_MESSAGE
};

enum class AlertState : uint8_t {
  // Jenis tanda bahaya yang bisa muncul.
  // Nilai ini dipakai untuk menyalakan alarm dan menulis catatan.
  Idle,
  AccessGranted,
  AccessDenied,
  Lockout,
  ThermalWarning,
  ThermalCritical
};

class App {
 public:
  // Objek ini mengurus semua bagian sistem dari awal sampai akhir.
  // Di sinilah semua pengurus lain disambungkan.
  App();

  // Bagian awal dan bagian yang terus berulang.
  // setup() dipanggil sekali, loop() dipanggil terus-menerus.
  void setup();
  void loop();

 private:
  // Pengurus setelan yang dibaca dari memori internal.
  // Ini menyimpan data yang tidak boleh hilang saat listrik mati.
  ConfigManager _config;
  // Pengurus sambungan WiFi dan mode jaringan.
  // Bagian ini memilih apakah alat ikut jaringan rumah atau jadi AP.
  WiFiManager _wifi;
  // Pengurus sensor suhu dan kelembapan.
  // Bagian ini membaca kondisi lingkungan.
  SensorManager _sensors;
  // Pengurus keypad dan akses pintu.
  // Bagian ini memeriksa PIN dan mencatat kejadian masuk.
  AccessController _access;
  // Pengurus halaman web dan pengiriman data.
  // Bagian ini melayani browser dan cloud.
  NetworkServices _network;
  // Pengurus layar LCD.
  // Ini yang menampilkan semua informasi ke layar depan.
  Display _display;
  // Pengunci agar data tidak ditulis bersamaan.
  // Dipakai saat ada tugas latar belakang.
  SemaphoreHandle_t _runtimeMutex = nullptr;
  // Tugas latar belakang untuk jaringan dan pengiriman data.
  // Ini membuat kerja utama tidak terlalu berat.
  TaskHandle_t _backgroundTask = nullptr;
  // Tugas khusus untuk pembaruan program.
  // Dipakai saat upload firmware.
  TaskHandle_t _otaTask = nullptr;
  bool _backgroundTaskEnabled = false;
  bool _otaTaskEnabled = false;

  // Status kipas dan alarm yang dipakai sistem.
  // Nilai ini disalin ke layar, jaringan, dan catatan.
  bool _fan1On = false;
  bool _fan2On = false;
  bool _warning = false;
  bool _solenoidOn = false;
  bool _otaReady = false;
  unsigned long _solenoidUnlockUntilMs = 0;
  bool _alertOn = false;
  bool _fan1RelayApplied = false;
  bool _fan2RelayApplied = false;
  bool _solenoidRelayApplied = false;
  bool _alertRelayApplied = false;
  AlertState _alertState = AlertState::Idle;
  uint8_t _thermalTier = 0;
  unsigned long _alertUntilMs = 0;

  // Keadaan menu dan input pengguna di layar.
  // Semua isian tombol dan pindah halaman disimpan di sini.
  UIState _uiState = UIState::MONITORING;
  String _pinBuf;
  String _confirmBuf;
  String _authUserId;
  String _authDisplayName;
  String _selectedUserId;
  String _autoUserId;
  uint8_t _changePinStep = 0;
  uint8_t _userListAction = 0;
  unsigned long _uiIdleMs = 0;
  unsigned long _unlockOkMs = 0;
  unsigned long _statusUntilMs = 0;
  unsigned long _lastApDisplayRefreshMs = 0;
  unsigned long _lastLockoutRefreshMs = 0;
  UIState _statusReturnState = UIState::MONITORING;

  // Batas waktu tampilan untuk berbagai pesan sementara.
  // Angka ini membuat pesan tidak tampil terlalu lama.
  static constexpr unsigned long UI_TIMEOUT_MS = 30000;
  static constexpr unsigned long UNLOCK_DISPLAY_MS = 3000;
  static constexpr unsigned long AP_DISPLAY_REFRESH_MS = 1000;
  static constexpr unsigned long LOCKOUT_REFRESH_MS = 1000;
  static constexpr uint8_t ALERT_QUEUE_SIZE = 6;

  // Antrian tanda bahaya agar muncul satu per satu.
  // Supaya alarm tidak saling menimpa.
  AlertState _alertQueue[ALERT_QUEUE_SIZE] = {};
  uint8_t _alertQueueHead = 0;
  uint8_t _alertQueueTail = 0;
  uint8_t _alertQueueCount = 0;

  // Salinan ringkas keadaan sistem untuk dibaca tugas lain.
  // Snapshot ini dipakai agar data dibaca dengan aman.
  struct RuntimeSnapshot {
    SensorData data{};
    bool fan1On = false;
    bool fan2On = false;
    bool warning = false;
    bool solenoidOn = false;
    bool alertOn = false;
    AlertState alertState = AlertState::Idle;
  } _runtimeSnapshot;

  // Fungsi bantu untuk menyiapkan bagian-bagian sistem.
  // Semua fungsi ini dipakai dari setup() dan loop().
  void setupOTA();
  void setupRelays();
  void startBackgroundTask();
  void startOtaTask();
  void publishRuntimeSnapshot(const SensorData& data);
  RuntimeSnapshot readRuntimeSnapshot();
  static void backgroundTaskEntry(void* context);
  static void otaTaskEntry(void* context);
  void backgroundTaskLoop();
  void otaTaskLoop();
  // Fungsi bantu untuk menentukan keadaan dan tampilan.
  // Bagian ini mengubah data sensor jadi keputusan nyata.
  void updateThermalAndFans(const SensorData& data);
  void updateSolenoid();
  void updateAlert();
  void handleAccessAlert(const AccessEvent& event);
  void updateThermalAlertState(bool warning, bool critical);
  void enqueueAlert(AlertState state);
  void clearQueuedAlerts(bool stopCurrent = true);
  bool dequeueAlert(AlertState& state);
  void startAlert(AlertState state);
  void stopAlert();
  void setAlertRelay(bool on);
  [[nodiscard]] bool hasContinuousThermalAlert() const;
  [[nodiscard]] uint16_t alertDurationMs(AlertState state) const;
  [[nodiscard]] const char* alertStateName() const;
  static const char* alertStateName(AlertState state);
  // Fungsi bantu untuk memasang relay dan input pengguna.
  // Relay dipakai untuk menggerakkan beban fisik.
  void requestUnlock();
  void startUnlockSession(const String& displayName);
  void setRelay(uint8_t pin, bool on);
  // Fungsi bantu untuk menggambar ulang layar.
  // Semua teks yang terlihat di LCD dirangkai di sini.
  void updateDisplay(const SensorData& data);
  void renderCurrentUiState();
  void showTransientMessage(const char* title, const char* msg, bool success,
                            UIState returnState, unsigned long durationMs);
  // Fungsi bantu untuk membaca tombol dan mengatur menu.
  // Semua gerak menu keypad lewat sini.
  void handleUIKey(char key);
  void resetToMonitoring();
  void buildUserSlotMap();
  void popLastDigit(String& buffer);

  // Peta slot pengguna untuk menu daftar.
  // Dipakai supaya daftar user bisa dipilih dengan angka.
  static constexpr uint8_t MAX_SLOTS = 10;
  uint8_t _userSlotCount = 0;
  uint8_t _userSlotMap[MAX_SLOTS] = {};
};
