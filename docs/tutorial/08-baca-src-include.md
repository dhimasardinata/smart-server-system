# 08 - Cara Membaca `src` dan `include`

Bab ini membantu kamu membaca file kode utama tanpa bingung.

## Cara Pikir Umum

File di `src/` biasanya berisi kerja utama.

File di `include/` biasanya berisi:

- daftar fungsi
- bentuk data
- nama alat

## File Paling Penting di `src`

### `src/main.cpp`

Ini pintu masuk program.

Yang perlu dicari:

- objek utama yang dipakai
- bagian yang dijalankan sekali
- bagian yang diulang terus

### `src/App.cpp`

Ini pusat alur kerja.

Yang perlu dicari:

- kapan semua alat disiapkan
- kapan sensor dibaca
- kapan kipas atau pintu diubah
- kapan layar diperbarui
- kapan data dikirim

### `src/Config.cpp`

Ini pengurus setelan.

Yang perlu dicari:

- dari mana data disimpan
- bagaimana data dibaca
- apa yang dilakukan kalau data rusak

### `src/Display.cpp`

Ini pengurus layar.

Yang perlu dicari:

- bagaimana layar dinyalakan
- teks apa yang ditampilkan
- kapan layar diperbarui

### `src/Sensors.cpp`

Ini pengurus sensor.

Yang perlu dicari:

- sensor mana yang dipakai
- bagaimana sensor dinyalakan
- bagaimana hasil dibaca
- apa yang dilakukan kalau sensor gagal

### `src/WiFiHandler.cpp`

Ini pengurus jaringan.

Yang perlu dicari:

- bagaimana jaringan dicari
- bagaimana sambungan dicoba
- kapan mode titik akses dinyalakan

### `src/AccessController.cpp`

Ini pengurus PIN dan akses.

Yang perlu dicari:

- bagaimana tombol dibaca
- bagaimana PIN dicek
- kapan akses diterima
- kapan akses ditolak
- kapan kunci sementara aktif

### `src/NetworkServices.cpp`

Ini pengurus halaman web dan kirim data.

Yang perlu dicari:

- jalur web apa yang disediakan
- data apa yang dikirim
- bagaimana firmware diterima

### `src/GoogleSheetsClient.cpp`

Ini pengirim data ke spreadsheet.

Yang perlu dicari:

- alamat tujuan
- format data yang dikirim
- apa yang terjadi kalau kirim gagal

### `src/OtaCoordinator.cpp`

Ini pengatur pembaruan program.

Yang perlu dicari:

- kapan pembaruan dimulai
- bagaimana statusnya disimpan
- bagaimana dua cara pembaruan dijaga supaya tidak bentrok

## File Penting di `include`

### `include/App.h`

Berisi rangka besar pengendali utama.

Di sini ada:

- daftar bagian yang dipakai
- keadaan layar
- keadaan tanda bahaya
- fungsi-fungsi penting yang dipanggil dari `App.cpp`

### `include/Config.h`

Berisi bentuk setelan dan data pengguna.

### `include/Display.h`

Berisi fungsi layar.

### `include/Sensors.h`

Berisi fungsi sensor.

### `include/WiFiHandler.h`

Berisi fungsi jaringan.

### `include/AccessController.h`

Berisi fungsi akses pintu.

### `include/NetworkServices.h`

Berisi fungsi web dan pengiriman data.

### `include/GoogleSheetsClient.h`

Berisi fungsi pengirim data ke spreadsheet.

### `include/OtaCoordinator.h`

Berisi fungsi pengatur pembaruan.

### `include/PinMap.h`

Berisi daftar pin supaya tidak perlu menulis angka yang sama di banyak tempat.

### `include/WebPage.h`

Berisi isi halaman web yang dikirim langsung dari ESP32.

## Cara Membaca File Dengan Aman

Kalau kamu membuka satu file, cari urutan ini:

1. data apa yang disimpan di atas
2. fungsi apa yang ada di tengah
3. bagian mana yang dipakai dari file lain
4. komentar yang menjelaskan alasan

## Tanda Yang Perlu Diperhatikan

- nama variabel yang mirip
- fungsi yang dipanggil berulang
- bagian yang memakai `millis()`
- bagian yang memakai `Serial.println(...)`
- bagian yang menyentuh pin atau layar

## Cara Belajar Yang Enak

Jangan baca semua baris sekaligus.

Pilih satu file, lalu jawab:

- file ini mengurus apa
- data apa yang dipakai
- kalau rusak, gejalanya seperti apa

## Latihan Kecil

Pilih satu file dari `src/`, lalu tulis:

- tugas utamanya
- file apa yang dia panggil
- alat apa yang dipakai
