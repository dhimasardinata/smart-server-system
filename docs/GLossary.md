# GLossary

Daftar istilah singkat yang sering muncul di proyek ini.

## Istilah Dasar

- `ESP32`: chip utama yang menjalankan seluruh sistem
- `Arduino`: cara kerja yang memudahkan menulis program untuk ESP32
- `sketch`: nama lain untuk program Arduino/ESP32
- `sensor`: alat yang membaca kondisi, misalnya suhu dan kelembapan
- `keypad`: papan tombol untuk memasukkan PIN
- `relay`: saklar elektrik yang dikendalikan oleh ESP32
- `solenoid`: kunci elektrik yang bisa membuka atau mengunci pintu
- `LCD`: layar kecil untuk menampilkan tulisan
- `cloud`: penyimpanan atau layanan yang ada di internet
- `dashboard`: halaman web untuk melihat data

## Istilah Sambungan

- `I2C`: cara sederhana untuk menghubungkan ESP32 dengan sensor atau layar
- `bus`: jalur bersama yang dipakai beberapa alat untuk bertukar data
- `SDA` dan `SCL`: dua jalur utama pada I2C
- `GPIO`: pin serbaguna pada ESP32
- `pin`: kaki sambungan pada ESP32
- `pinMode`: cara menyiapkan pin untuk input atau output
- `digitalWrite`: cara menyalakan atau mematikan pin
- `Serial`: jalur untuk mengirim teks ke laptop
- `SSID`: nama jaringan WiFi
- `AP mode`: mode saat ESP32 membuat jaringan WiFi sendiri
- `mDNS`: cara agar perangkat bisa dibuka lewat nama, bukan hanya alamat angka

## Istilah Program

- `boot`: saat alat baru menyala
- `loop`: bagian program yang terus berulang
- `setup`: bagian yang dijalankan satu kali saat alat menyala
- `pembaruan`: mengganti program lama dengan program baru
- `pembaruan lewat kabel`: update program lewat sambungan USB
- `pembaruan lewat jaringan`: update program tanpa kabel USB
- `penguncian sementara`: keadaan saat akses ditahan karena terlalu banyak salah PIN
- `catatan`: data kejadian yang disimpan agar bisa dilihat lagi
- `delay`: jeda singkat sebelum lanjut ke langkah berikutnya
- `millis`: hitung berapa lama alat sudah hidup
- `Serial Monitor`: layar di komputer untuk melihat tulisan dari ESP32

## Istilah Penyimpanan

- `LittleFS`: tempat simpan file kecil di dalam ESP32
- `config`: pengaturan yang dipakai program
- `default`: nilai bawaan saat belum ada pengaturan sendiri

## Istilah Jaringan

- `alamat tujuan`: alamat tempat data dikirim
- `web`: halaman yang dibuka dari browser
- `spreadsheet`: lembar data online yang mirip tabel
- `NTP`: cara mengambil waktu dari internet supaya jam di perangkat tepat
- `HTTP`: cara umum untuk meminta dan mengirim data lewat web
- `HTTPS`: versi aman dari HTTP
- `JSON`: bentuk data yang sering dipakai untuk kirim-terima data
- `TLS`: lapisan keamanan saat data dikirim lewat internet

## Istilah Akses

- `PIN`: kode angka untuk membuka akses
- `lockout`: penguncian sementara setelah terlalu banyak gagal
- `admin`: pengguna yang punya hak lebih besar
- `user`: pengguna biasa yang boleh masuk sesuai izin

## Istilah C++

- `compiler`: alat penerjemah yang mengubah kode C++ jadi program yang bisa dijalankan
- `variabel`: kotak untuk menyimpan nilai
- `fungsi`: kumpulan langkah yang bisa dipanggil ulang
- `class`: cetakan untuk membuat benda dengan data dan aksi
- `struct`: bentuk sederhana untuk mengelompokkan data
- `pointer`: alamat tempat data berada
- `referensi`: nama lain untuk data yang sama
- `template`: pola kode yang bisa dipakai ulang untuk banyak tipe data
- `namespace`: nama pembungkus agar nama tidak saling tabrakan
- `const`: tanda bahwa nilai tidak boleh diubah
- `constexpr`: nilai yang bisa dihitung saat program disiapkan
- `RAII`: cara mengatur sumber daya supaya otomatis rapi saat keluar dari bagian kode
- `span`: cara melihat potongan data tanpa menyalin
- `expected`: cara membawa hasil yang bisa berhasil atau gagal
- `ranges`: cara mengolah data dengan alur yang lebih ringkas

## Istilah Perangkat Lunak Lain

- `FreeRTOS`: pengatur kerja banyak tugas di ESP32
- `OTA`: pembaruan program lewat jaringan
- `firmware`: program yang ditanam di perangkat
- `host`: nama atau alamat yang dipakai untuk menemukan perangkat
- `timeout`: batas waktu tunggu sebelum dianggap gagal
