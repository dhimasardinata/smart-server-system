# 02 - File Proyek

Bab ini menjelaskan isi folder proyek.

## `src`

Di sini ada program utama yang berjalan di ESP32.

Isi paling penting biasanya ada di sini karena di sinilah alat bekerja.

## `include`

Di sini ada file bantuan yang dipakai oleh program utama.

File di sini biasanya berisi bentuk data atau daftar fungsi yang dipakai bersama.

## `web-dashboard`

Di sini ada halaman web untuk melihat data dari jauh.

Kalau dibuka di browser, folder ini jadi tampilan untuk pengguna.

## `google-apps-script`

Di sini ada skrip penerima data untuk spreadsheet online.

Bagian ini membuat data dari ESP32 bisa masuk ke tabel online.

## `tests`

Di sini ada skrip contoh untuk mengirim data uji.

Tujuannya untuk mengecek apakah jalur penting masih aman.

## `platformio.ini`

File ini mengatur cara program dibangun dan alat apa saja yang dipakai.

Kalau ingin tahu perpustakaan apa yang dipakai, biasanya lihat bagian ini dulu.

## Urutan Baca

Kalau ingin paham cepat:

1. `src/main.cpp`
2. `src/App.cpp`
3. `include/*.h`
4. `web-dashboard/app.js`
5. `google-apps-script/Code.gs`

## Cara Melihat Hubungan Folder

Bayangkan proyek ini seperti rumah:

- `src/` adalah ruang kerja utama
- `include/` adalah rak berisi catatan penting
- `web-dashboard/` adalah ruang lihat dari jauh
- `google-apps-script/` adalah kotak penerima data ke spreadsheet
- `tests/` adalah alat cek

## Latihan Kecil

Pilih satu folder, lalu jawab:

- folder ini mengurus apa
- file apa di dalamnya yang paling penting
- kalau file ini rusak, apa yang mungkin ikut bermasalah
