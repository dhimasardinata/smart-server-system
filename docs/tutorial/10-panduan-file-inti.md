# 10 - Panduan Membaca File Inti

Bab ini membantu kamu membaca file inti proyek dari atas ke bawah.

Tujuannya bukan menghafal semua baris, tapi tahu bagian mana yang mengurus apa.

## Cara Umum Membaca

Kalau membuka satu file, cari urutan ini:

1. daftar `#include`
2. bagian `namespace` atau nilai tetap
3. fungsi bantu
4. kelas atau objek utama
5. fungsi yang dipanggil dari luar

## `src/main.cpp`

File ini sangat pendek.

Yang penting:

- membuat objek utama `App`
- memanggil `app.setup()`
- memanggil `app.loop()`

Artinya:

- `setup()` dijalankan sekali
- `loop()` dijalankan terus-menerus

## `src/App.cpp`

Ini pusat pengendali.

Bagian awal file:

- mengatur pin relay
- mengatur nama perangkat untuk jaringan
- menyiapkan nilai bawaan untuk relay dan status

Bagian `setup()`:

- menyiapkan saklar
- membuka jalur Serial
- menyiapkan jalur I2C
- memuat setelan
- menyalakan layar
- menyiapkan sensor
- menyiapkan akses keypad
- menyiapkan WiFi
- menyiapkan layanan jaringan
- menyiapkan pembaruan lewat jaringan
- membaca data awal
- memulai tugas latar belakang

Bagian `setupOTA()`:

- menyiapkan nama perangkat di jaringan
- menyiapkan pembaruan lewat jaringan
- memberi kabar saat pembaruan mulai, berjalan, selesai, atau gagal

Bagian `updateThermalAndFans()`:

- melihat suhu dan kelembapan
- menentukan apakah peringatan aktif
- menghidupkan kipas pertama dan kedua

Bagian `requestUnlock()` dan `startUnlockSession()`:

- membuka solenoid sebentar
- menampilkan pesan bahwa akses diterima

Bagian `updateSolenoid()`:

- mengecek apakah waktu buka sudah habis
- menutup kunci lagi kalau waktunya selesai

Bagian `setAlertRelay()`:

- menyalakan atau mematikan relay alarm

Bagian `loop()`:

- menjalankan semua pembaruan berulang
- membaca sensor
- mengatur layar
- mengatur WiFi
- mengatur akses
- mengatur pengiriman data

## `src/Config.cpp`

Ini pengurus setelan.

Bagian pentingnya:

- nilai bawaan disiapkan di `AppConfig::AppConfig()`
- `ConfigManager::begin()` membuka penyimpanan `LittleFS`
- `ConfigManager::load()` membaca file setelan
- `ConfigManager::save()` menyimpan setelan
- `ConfigManager::formatFileSystem()` merapikan memori kalau rusak

Yang perlu dicari:

- data lama yang masih diterima
- nama kunci baru dan lama
- nilai bawaan untuk WiFi, pengguna, sensor, dan batas suhu

## `src/Display.cpp`

Ini pengurus layar.

Bagian penting:

- `initialize()` menyiapkan layar
- `begin()` memulai layar
- `maintainConnection()` mengecek apakah layar masih hidup
- `showMainScreen()` menampilkan isi utama
- `showPinEntry()` menampilkan halaman masuk PIN
- `showUnlockOk()` menampilkan akses diterima
- `showAdminMenu()` dan fungsi lain menampilkan menu pengguna

Yang perlu dicari:

- kapan layar dihapus
- kapan layar ditulis ulang
- bagaimana baris pertama bergeser pelan

## `src/Sensors.cpp`

Ini pengurus sensor.

Bagian penting:

- `logI2cScan()` mengecek alat yang menempel di jalur I2C
- `SHT21Sensor::initialize()` menyiapkan sensor SHT21
- `SHT3xSensor::initialize()` menyiapkan sensor SHT3x
- `read()` membaca suhu dan kelembapan
- `SensorManager::begin()` menyiapkan semua sensor
- `SensorManager::update()` membaca sensor dengan jarak waktu tertentu

Yang perlu dicari:

- apa yang dilakukan saat sensor tidak ditemukan
- kapan sensor dicoba lagi
- bagaimana data ditandai valid atau tidak

## `src/WiFiHandler.cpp`

Ini pengurus jaringan.

Bagian penting:

- `begin()` memilih mode jaringan awal
- `startScan()` mencari jaringan
- `processScanResults()` mengurutkan jaringan yang ditemukan
- `tryNextNetwork()` mencoba sambungan satu per satu
- `verifyInternet()` mengecek apakah internet benar-benar aktif
- `startApMode()` menyalakan jaringan sementara untuk penyetelan

Yang perlu dicari:

- kapan mode WiFi diubah
- kapan AP mode dipakai
- kapan sambungan dianggap berhasil

## `src/AccessController.cpp`

Ini pengurus PIN dan akses.

Bagian penting:

- `hashPinSha256()` mengubah PIN jadi jejak aman
- `isValidPinFormat()` mengecek PIN angka dan panjangnya
- `begin()` menyiapkan keypad
- `validatePin()` mengecek PIN yang dimasukkan
- `changePin()` mengganti PIN pengguna
- `generateUserId()` membuat nama pengguna baru
- `update()` mengecek apakah penguncian sementara selesai

Yang perlu dicari:

- kapan akses diterima
- kapan akses ditolak
- kapan penguncian sementara dimulai

## `src/NetworkServices.cpp`

Ini pengurus halaman web dan pengiriman data.

Bagian penting:

- fungsi bantu untuk jalur API
- `begin()` menyalakan server web dan tugas upload
- `update()` menyalin data penting ke cache
- `logAccessEvent()` mengubah catatan akses menjadi data kirim
- `enqueueTelemetryPayload()` dan `enqueueAccessPayload()` menaruh data ke antrean
- `uploadTaskLoop()` mengirim data saat aman
- `handleRoot()` dan fungsi `handle...()` menerima permintaan dari browser
- `handleOtaUploadChunk()` menerima file firmware

Yang perlu dicari:

- jalur mana untuk lihat data
- jalur mana untuk ubah setelan
- jalur mana untuk upload firmware

## `src/GoogleSheetsClient.cpp`

Ini pengirim data ke spreadsheet.

Bagian penting:

- `urlEncode()` mengubah teks agar aman dikirim lewat alamat web
- `begin()` menyimpan alamat tujuan
- `sendGetRequest()` mengirim permintaan
- `sendTelemetry()` menyiapkan data suhu dan kelembapan
- `sendAccess()` menyiapkan data akses pintu

Yang perlu dicari:

- nama data apa yang dikirim
- kapan data dianggap gagal
- bagaimana pesan gagal disimpan

## `src/OtaCoordinator.cpp`

Ini pengatur pembaruan program.

Bagian penting:

- `begin()` menyiapkan pengunci
- `beginArduino()` menyalakan mode pembaruan lewat kabel
- `beginWeb()` menyalakan mode pembaruan lewat web
- `updateArduinoProgress()` dan `updateWebProgress()` menyimpan kemajuan
- `finishArduino()` dan `finishWeb()` menutup mode pembaruan
- `snapshot()` memberi potret singkat status

Yang perlu dicari:

- kapan pembaruan boleh jalan
- kapan pembaruan ditolak
- bagaimana status ditampilkan ke web

## `include/App.h`

File ini penting karena berisi peta besar dari `App`.

Yang ada di dalamnya:

- keadaan tampilan
- keadaan peringatan
- data cache
- fungsi-fungsi pengendali

Kalau ingin tahu apa saja yang dikendalikan sistem, file ini tempat yang baik untuk mulai.

## `include/Config.h`

Berisi:

- bentuk WiFi
- bentuk pengguna
- bentuk setelan utama
- nama kunci penyimpanan

## `include/Display.h`

Berisi:

- pengurus layar
- fungsi tampil dan tulis
- data yang ditampilkan

## `include/Sensors.h`

Berisi:

- bentuk data sensor
- pengurus sensor SHT21
- pengurus sensor SHT3x
- pengurus pembaca sensor

## `include/WiFiHandler.h`

Berisi:

- keadaan WiFi
- daftar jaringan hasil pindai
- fungsi mencari, menyambung, dan masuk AP mode

## `include/AccessController.h`

Berisi:

- jenis kejadian akses
- bentuk catatan akses
- hasil cek PIN
- pengurus keypad dan PIN

## `include/NetworkServices.h`

Berisi:

- pengurus layanan web
- antrean data telemetry dan akses
- fungsi handler untuk jalur API

## `include/GoogleSheetsClient.h`

Berisi:

- bentuk data telemetry
- bentuk data akses
- fungsi kirim ke spreadsheet

## `include/OtaCoordinator.h`

Berisi:

- mode pembaruan
- potret status pembaruan
- fungsi pengatur pembaruan

## `include/PinMap.h`

Berisi:

- pin I2C
- pin relay
- pin keypad
- aturan aktif tinggi atau aktif rendah

## `include/WebPage.h`

Berisi isi HTML web yang disimpan langsung di firmware.

## Cara Belajar File Inti

Kalau kamu mau cepat paham file inti:

1. baca header dulu
2. lihat fungsi yang paling sering dipanggil
3. cari komentar yang menjelaskan alasan
4. jangan panik dengan nama yang panjang

## Latihan Kecil

Ambil satu file inti, lalu jawab:

- file ini mengurus apa
- fungsi mana yang paling penting
- data apa yang disimpan
- bagian mana yang paling mungkin berubah saat fitur baru ditambah
