# 13 - Peta Kode dan Pustaka Proyek

Bab ini adalah peta besar untuk seluruh proyek.

Kalau kamu membaca bab ini, tujuanmu bukan menghafal, tapi tahu:

1. file mana mengurus apa
2. pustaka mana dipakai untuk tugas apa
3. urutan belajar yang paling masuk akal

## Gambaran Besar

Proyek ini bekerja seperti ini:

```text
sensor -> ESP32 -> layar / relay / keamanan -> jaringan -> cloud / spreadsheet
```

Artinya:

- sensor membaca keadaan ruangan
- ESP32 mengolah data
- layar menampilkan hasil
- relay menghidupkan atau mematikan alat
- keypad dipakai untuk masuk
- jaringan dipakai untuk web dan kirim data
- cloud dipakai untuk catatan jarak jauh

## Bagian Kode Utama

### `src/main.cpp`

Ini pintu masuk program.

Di sini ada dua bagian yang paling penting:

- bagian yang dijalankan sekali saat alat menyala
- bagian yang diulang terus selama alat hidup

### `src/App.cpp`

Ini pusat pengendali.

Fungsinya:

- menyalakan semua bagian
- mengatur alur kerja utama
- membaca keadaan sensor
- mengatur kipas, pintu, dan alarm
- menjaga layar tetap mengikuti keadaan terbaru

### `src/Config.cpp`

Ini pengatur simpan-muat setelan.

Fungsinya:

- membaca setelan dari penyimpanan internal
- menyimpan setelan baru
- mengembalikan setelan bawaan kalau data rusak

### `src/Display.cpp`

Ini pengurus layar.

Fungsinya:

- menyalakan layar
- menulis teks ke layar
- menampilkan suhu, kelembapan, jaringan, dan status pintu

### `src/Sensors.cpp`

Ini pengurus sensor suhu dan kelembapan.

Fungsinya:

- menyiapkan sensor
- membaca data sensor
- mencoba lagi kalau sensor bermasalah

### `src/I2CBus.cpp`

Ini pengurus jalur bersama untuk alat yang pakai komunikasi dua kabel.

Fungsinya:

- menyiapkan jalur data
- mencoba membuka jalur kalau macet
- membantu layar dan sensor tetap bisa dipakai

### `src/WiFiHandler.cpp`

Ini pengurus jaringan.

Fungsinya:

- mencari jaringan yang tersimpan
- mencoba tersambung
- menyalakan mode titik akses kalau perlu
- memeriksa apakah internet benar-benar bisa dipakai

### `src/AccessController.cpp`

Ini pengurus pintu masuk.

Fungsinya:

- membaca tombol keypad
- mengecek PIN
- mengatur siapa yang boleh masuk
- menghitung salah PIN
- mengaktifkan penguncian sementara

### `src/NetworkServices.cpp`

Ini pengurus layanan jaringan.

Fungsinya:

- menyiapkan halaman web
- menerima pengaturan dari web
- mengirim data ke spreadsheet online
- mengatur upload pembaruan program lewat web

### `src/GoogleSheetsClient.cpp`

Ini pengirim data ke spreadsheet online.

Fungsinya:

- menyiapkan alamat tujuan
- mengirim data suhu, kelembapan, dan akses pintu
- menyimpan status kirim terakhir

### `src/OtaCoordinator.cpp`

Ini pengatur pembaruan program.

Fungsinya:

- membedakan pembaruan lewat kabel dan lewat web
- menjaga supaya pembaruan tidak bentrok
- menyimpan info kemajuan pembaruan

## Header Yang Menjelaskan Bentuk Data

File header adalah file yang biasanya berisi bentuk data, nama alat, dan daftar fungsi yang dipakai dari luar.

### `include/PinMap.h`

Berisi daftar pin ESP32 yang dipakai proyek.

Ini membantu supaya angka pin tidak tercecer di banyak tempat.

### `include/Config.h`

Berisi bentuk setelan dan pengelola setelan.

Di sini ada:

- daftar jaringan WiFi
- daftar pengguna
- batas suhu
- batas kelembapan
- setelan penguncian
- alamat layanan spreadsheet

### `include/Display.h`

Berisi bentuk pengurus layar.

### `include/Sensors.h`

Berisi bentuk pengurus sensor.

### `include/WiFiHandler.h`

Berisi bentuk pengurus jaringan.

### `include/AccessController.h`

Berisi bentuk pengurus akses pintu.

### `include/NetworkServices.h`

Berisi bentuk pengurus layanan jaringan dan data kirim.

### `include/GoogleSheetsClient.h`

Berisi bentuk pengirim data ke spreadsheet online.

### `include/OtaCoordinator.h`

Berisi bentuk pengatur pembaruan program.

### `include/WebPage.h`

Berisi halaman web yang disimpan langsung di perangkat.

## Folder Web

### `web-dashboard/index.html`

Ini tampilan utama halaman web di komputer atau ponsel.

### `web-dashboard/styles.css`

Ini pengatur warna, jarak, dan tampilan halaman.

### `web-dashboard/app.js`

Ini pengatur perilaku halaman.

Fungsinya:

- memanggil data dari ESP32
- mengisi form
- mengirim setelan
- mengunggah firmware

### `web-dashboard/netlify.toml`

Ini setelan untuk meng-host halaman web bila dipakai di Netlify.

## Folder Google Apps Script

### `google-apps-script/Code.gs`

Ini skrip yang menerima data dari ESP32 lalu menaruhnya ke spreadsheet.

### `google-apps-script/.clasp.json`

Ini catatan sambungan untuk alat sinkronisasi Google Apps Script.

### `google-apps-script/.claspignore`

Ini daftar file yang tidak ikut dikirim saat sinkronisasi.

## Folder Tests

Folder `tests/` berisi skrip cek.

Tujuannya:

- memastikan jalur penting tidak rusak
- mengecek hasil akses
- mengecek hasil telemetri

Contoh file:

- `tests/run_all.py`
- `tests/test_access_granted.py`
- `tests/test_access_denied.py`
- `tests/test_telemetry_normal.py`
- `tests/test_telemetry_warning.py`
- `tests/test_telemetry_alarm.py`

## Pengaturan Build

### `platformio.ini`

Ini papan aturan saat proyek dibangun.

Di sini diatur:

- jenis papan ESP32
- jenis kerja yang dipakai
- pustaka yang perlu diambil
- ukuran file sistem internal
- ukuran pembagian memori
- batas standar C++ yang dipakai
- cara upload biasa dan upload lewat jaringan

### `tools/setup_toolchain.py`

Ini skrip bantu yang menyiapkan alat terjemah C++ untuk ESP32.

Sederhananya:

- kalau alat belum ada, skrip ini mengambilnya
- kalau sudah ada, skrip ini memakainya
- kalau ada versi lama, skrip ini mencoba memindahkan dengan aman

### `tools/setup_esptool.py`

Ini skrip bantu untuk alat pengirim program ke ESP32.

Sederhananya:

- kalau alat pengirim belum ada, skrip ini mengambilnya
- kalau ada versi lama, skrip ini mencoba memindahkannya
- kalau sudah siap, skrip ini menyiapkan jalur upload

### `board_build.partitions` dan `board_build.filesystem`

Ini bagian yang menentukan cara memori dibagi.

Di proyek ini dipakai:

- `littlefs` untuk simpan file kecil
- partisi kecil yang cocok untuk kebutuhan proyek

## Jalur Upload

Proyek ini punya dua cara upload:

- lewat kabel
- lewat jaringan

Kalau upload lewat jaringan dipakai, perangkat akan menerima file baru tanpa harus dicolok kabel.

## Pustaka Yang Dipakai

Bagian ini menjelaskan pustaka yang dipakai proyek, dengan bahasa sederhana.

## Cara Pakai Arduino

`Arduino.h` adalah dasar dari hampir semua file C++ di proyek ini.

Yang paling sering dipakai:

- `Serial.begin(115200)` untuk membuka jalur tulisan ke laptop
- `Serial.print(...)` dan `Serial.println(...)` untuk memberi kabar
- `delay(ms)` untuk jeda singkat
- `millis()` untuk tahu berapa lama alat sudah hidup
- `pinMode(pin, OUTPUT)` atau `pinMode(pin, INPUT_PULLUP)` untuk menyiapkan pin
- `digitalWrite(pin, HIGH/LOW)` untuk menyalakan atau mematikan alat
- `isDigit(ch)` untuk mengecek angka pada keypad
- `String` untuk teks yang mudah dipakai di ESP32
- `snprintf(...)` untuk menyusun tulisan singkat dengan rapi
- `F("teks")` untuk menyimpan teks tetap tanpa boros memori

Pola paling dasar:

```cpp
void setup() {
    Serial.begin(115200);
    pinMode(5, OUTPUT);
}

void loop() {
    digitalWrite(5, HIGH);
    delay(1000);
    digitalWrite(5, LOW);
    delay(1000);
}
```

## Cara Pakai Jalur Dua Kabel

Dipakai oleh layar dan sensor.

Pustaka yang muncul:

- `Wire.h`
- `LiquidCrystal_I2C.h`
- `SHTSensor.h`

Pola pakainya:

1. jalur disiapkan dulu
2. alat dicek apakah benar-benar ada
3. alat diaktifkan
4. data dibaca atau ditampilkan

Contoh jalur:

```cpp
Wire.begin();
Wire.beginTransmission(0x40);
Wire.endTransmission(true);
```

Contoh layar:

```cpp
LiquidCrystal_I2C lcd(0x27, 20, 4);
lcd.init();
lcd.backlight();
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Halo");
```

Contoh sensor:

```cpp
SHTSensor sensor(SHTSensor::SHT3X);
sensor.init(Wire);
sensor.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);
sensor.readSample();
float suhu = sensor.getTemperature();
float hum = sensor.getHumidity();
```

## Cara Pakai WiFi

Pustaka yang dipakai:

- `WiFi.h`
- `DNSServer.h`
- `HTTPClient.h`
- `WiFiClientSecure.h`
- `ESPmDNS.h`

Pola pakainya:

1. pilih mode jaringan
2. cari jaringan yang ada
3. coba sambung
4. cek apakah internet benar-benar bisa dipakai
5. kalau gagal, aktifkan mode titik akses

Contoh:

```cpp
WiFi.mode(WIFI_STA);
WiFi.setAutoReconnect(true);
WiFi.begin("nama-wifi", "kata-sandi");
```

Kalau mau membuat mode titik akses:

```cpp
WiFi.softAP("TempMonitor-Setup");
IPAddress ip = WiFi.softAPIP();
```

`DNSServer` dipakai supaya alamat tertentu bisa diarahkan ke ESP32:

```cpp
dns.start(53, "*", ip);
dns.processNextRequest();
dns.stop();
```

`HTTPClient` dipakai untuk cek sambungan internet dan kirim data:

```cpp
HTTPClient http;
http.setTimeout(5000);
http.begin("https://contoh");
int code = http.GET();
http.end();
```

`WiFiClientSecure` dipakai kalau pengiriman butuh jalur aman:

```cpp
WiFiClientSecure client;
client.setInsecure();
```

`ESPmDNS` dipakai supaya perangkat bisa dicari dengan nama:

```cpp
MDNS.begin("monitor-server");
```

## Cara Pakai Penyimpanan

Pustaka yang dipakai:

- `LittleFS.h`
- `ArduinoJson.h`

`LittleFS` dipakai untuk menyimpan file kecil di dalam ESP32.

Pola pakainya:

```cpp
LittleFS.begin(false);
File f = LittleFS.open("/config.json", "r");
```

Kalau file belum ada atau rusak, proyek ini membuat ulang setelan bawaan.

`ArduinoJson` dipakai untuk:

- membaca data setelan
- menyimpan data setelan
- membalas data lewat web

Pola pakainya:

```cpp
JsonDocument doc;
deserializeJson(doc, file);
serializeJson(doc, response);
```

## Cara Pakai Keamanan PIN

Pustaka yang dipakai:

- `Keypad.h`
- `mbedtls/sha256.h`

`Keypad` dipakai untuk membaca tombol satu per satu.

Pola pakainya:

```cpp
// Bentuk keypad dari baris dan kolom.
Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, rows, cols);
// Ambil tombol yang sedang ditekan.
char key = keypad.getKey();
if (key != NO_KEY) {
    // ada tombol ditekan
}
```

`mbedtls/sha256.h` dipakai untuk mengubah PIN menjadi jejak aman sebelum disimpan.

Pola pakainya:

```cpp
mbedtls_sha256_context ctx;
mbedtls_sha256_init(&ctx);
mbedtls_sha256_starts(&ctx, 0);
mbedtls_sha256_update(&ctx, data, len);
mbedtls_sha256_finish(&ctx, hash);
mbedtls_sha256_free(&ctx);
```

## Cara Pakai Web Server

Pustaka yang dipakai:

- `ESPAsyncWebServer.h`
- `Update.h`
- `ArduinoOTA.h`

`ESPAsyncWebServer` dipakai untuk membuat halaman dan jalur layanan yang tetap responsif.

Pola umum:

```cpp
AsyncWebServer server(80);
server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{}");
});
server.begin();
```

Untuk kirim data JSON, proyek ini memakai aliran jawaban:

```cpp
AsyncResponseStream* response = request->beginResponseStream("application/json");
serializeJson(doc, *response);
request->send(response);
```

`Update.h` dipakai saat menerima file firmware baru.

`ArduinoOTA.h` dipakai untuk pembaruan lewat jaringan tanpa buka halaman web.

Contoh alurnya:

```cpp
ArduinoOTA.setHostname("monitor-server");
ArduinoOTA.onStart([]() {});
ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
ArduinoOTA.onEnd([]() {});
ArduinoOTA.begin();
```

## Cara Pakai Waktu dan Tugas Latar Belakang

Pustaka yang dipakai:

- `time.h`
- `freertos/FreeRTOS.h`
- `freertos/semphr.h`
- `freertos/task.h`

`time.h` dipakai untuk:

- menyelaraskan jam
- membuat cap waktu
- membaca waktu lokal

Contoh:

```cpp
configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
struct tm timeinfo;
getLocalTime(&timeinfo);
```

FreeRTOS dipakai untuk pekerjaan yang berjalan di belakang layar.

Contoh yang muncul di proyek ini:

- membuat pengunci agar data tidak ditulis bersamaan
- membuat tugas khusus untuk pengiriman cloud
- membangunkan tugas saat ada data baru

Contoh dasar:

```cpp
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
xSemaphoreTake(mutex, pdMS_TO_TICKS(20));
xSemaphoreGive(mutex);
```

## Cara Pakai Alat Tingkat Rendah

Pustaka yang dipakai:

- `driver/gpio.h`
- `esp_log.h`
- `esp_task_wdt.h`
- `Esp.h`

`driver/gpio.h` dipakai untuk menyiapkan pin output dengan cara yang lebih langsung.

Contoh:

```cpp
gpio_reset_pin(GPIO_NUM_5);
gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_5, 1);
```

`esp_log.h` dipakai untuk menurunkan suara catatan bawaan kalau terlalu ramai.

`esp_task_wdt.h` dipakai untuk mematikan pengawas tugas jika proyek ini mengelola tugas sendiri.

`Esp.h` dipakai untuk memanggil restart saat perlu.

## Cara Pakai Pustaka C++

Pustaka bawaan C++ yang dipakai:

- `cstdint`
- `algorithm`
- `array`
- `deque`
- `vector`
- `memory`
- `string_view`
- `type_traits`
- `concepts`
- `span`
- `expected`
- `ranges`

Contoh pemakaian dari proyek ini:

- `std::array` untuk daftar tetap
- `std::deque` untuk antrian kejadian
- `std::vector` untuk daftar hasil scan WiFi
- `std::unique_ptr` dan `std::make_unique` untuk objek yang dikelola otomatis
- `std::string_view` untuk membaca teks tanpa menyalin
- `std::sort` dan `std::min` untuk menyusun dan membatasi nilai
- `std::span` untuk melihat potongan data
- `std::expected` untuk hasil yang bisa berhasil atau gagal
- `std::views::filter` dari `ranges` untuk menyaring data

Contoh singkat:

```cpp
// Data tetap, jumlahnya tidak berubah.
std::array<int, 3> angka{1, 2, 3};
// Wadah data yang bisa bertambah.
std::vector<int> daftar;
// Antrian data untuk catatan atau tugas.
std::deque<String> antrean;
// Susun angka dari kecil ke besar.
std::sort(angka.begin(), angka.end());
```

## Cara Pakai Yang Paling Dekat Dengan Proyek Ini

Kalau dilihat dari alur nyata proyek ini, urutan pakainya sering seperti ini:

1. `Arduino.h` menyiapkan dasar
2. `PinMap.h` menentukan pin
3. `Config.h` dan `Config.cpp` memuat setelan
4. `I2CBus.h` dan `I2CBus.cpp` menyiapkan jalur alat
5. `Display.h` dan `Display.cpp` menyalakan layar
6. `Sensors.h` dan `Sensors.cpp` membaca suhu dan kelembapan
7. `WiFiHandler.h` dan `WiFiHandler.cpp` mencari jaringan
8. `AccessController.h` dan `AccessController.cpp` membaca keypad
9. `NetworkServices.h` dan `NetworkServices.cpp` menyiapkan web dan kirim data
10. `GoogleSheetsClient.h` dan `GoogleSheetsClient.cpp` mengirim data
11. `OtaCoordinator.h` dan `OtaCoordinator.cpp` menjaga pembaruan program
12. `App.h` dan `App.cpp` menyatukan semuanya

## Ringkas Cara Membaca Pustaka

Kalau kamu melihat nama pustaka baru, tanyakan:

- ini dipakai untuk baca, tulis, atau simpan?
- ini dipakai di awal sekali atau terus-menerus?
- ini dipakai di sensor, layar, jaringan, atau keamanan?

Kalau tiga pertanyaan itu terjawab, biasanya kamu sudah cukup paham arah pakainya.

## Urutan Belajar Yang Aman

Kalau kamu mau paham proyek ini dari nol, ikuti urutan ini:

1. [01-dasar-dan-cara-berpikir.md](./01-dasar-dan-cara-berpikir.md)
2. [02-variabel-dan-tipe-data.md](./02-variabel-dan-tipe-data.md)
3. [03-logika-dan-perulangan.md](./03-logika-dan-perulangan.md)
4. [04-fungsi-dan-ruang-kerja.md](./04-fungsi-dan-ruang-kerja.md)
5. [05-teks-dan-wadah-data.md](./05-teks-dan-wadah-data.md)
6. [06-struct-dan-class.md](./06-struct-dan-class.md)
7. [07-pointer-dan-referensi.md](./07-pointer-dan-referensi.md)
8. [08-oop.md](./08-oop.md)
9. [09-template-dan-generik.md](./09-template-dan-generik.md)
10. [10-cpp23-modern.md](./10-cpp23-modern.md)
11. [11-error-dan-debug.md](./11-error-dan-debug.md)
12. [12-mini-proyek.md](./12-mini-proyek.md)
13. [13-peta-kode-dan-pustaka.md](./13-peta-kode-dan-pustaka.md)

## Latihan Cara Membaca Kode

Ambil satu file source, lalu jawab tiga pertanyaan:

- file ini mengurus apa
- data apa yang dipakai
- kalau rusak, bagian mana yang paling dulu saya curigai

Kalau kamu bisa menjawab tiga hal itu, kamu sudah jauh lebih siap membaca proyek ini sendiri.
