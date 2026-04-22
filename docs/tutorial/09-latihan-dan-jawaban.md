# 09 - Latihan dan Jawaban

Bab ini berisi latihan singkat untuk semua materi, lalu jawaban singkatnya.

Tujuannya bukan menguji kamu keras-keras, tapi membantu kamu mengulang dengan tenang.

## Cara Pakai

1. Baca satu bab dulu.
2. Coba jawab latihannya sendiri.
3. Baru buka jawaban singkatnya.
4. Kalau belum cocok, baca ulang bab yang terkait.

## 01 - Dasar dan Cara Berpikir

### Latihan

1. Jelaskan dengan kata-katamu sendiri apa itu program.
2. Sebutkan tiga langkah alur dasar program.
3. Apa tugas `setup()` di ESP32?

### Jawaban Singkat

1. Program adalah kumpulan perintah yang dijalankan alat.
2. Baca data, olah data, keluarkan hasil.
3. `setup()` dijalankan satu kali saat ESP32 baru menyala.

## 02 - Variabel dan Tipe Data

### Latihan

1. Apa gunanya variabel?
2. Bedakan angka bulat dan angka pecahan.
3. Kapan memakai `bool`?

### Jawaban Singkat

1. Variabel dipakai untuk menyimpan nilai.
2. Angka bulat tidak punya koma, angka pecahan punya koma.
3. `bool` dipakai untuk dua pilihan, seperti ya/tidak atau nyala/mati.

## 03 - Logika dan Perulangan

### Latihan

1. Kapan memakai `if`?
2. Kapan memakai `switch`?
3. Kapan memakai `for`?

### Jawaban Singkat

1. Saat ingin memilih satu dari dua keadaan.
2. Saat pilihannya banyak tetapi tetap.
3. Saat jumlah ulangannya sudah jelas.

## 04 - Fungsi dan Ruang Kerja

### Latihan

1. Kenapa fungsi penting?
2. Apa itu ruang kerja?
3. Kenapa kode besar sebaiknya dipecah?

### Jawaban Singkat

1. Supaya kode bisa dipakai ulang dan lebih rapi.
2. Ruang kerja adalah batas tempat data bisa dipakai.
3. Supaya lebih mudah dibaca, dicari, dan diperbaiki.

## 05 - Teks dan Wadah Data

### Latihan

1. Apa beda `std::string` dan `std::string_view`?
2. Kapan memakai `std::array`?
3. Kapan memakai `std::vector`?

### Jawaban Singkat

1. `std::string` dipakai untuk teks yang bisa berubah, `string_view` hanya melihat teks.
2. `std::array` dipakai kalau jumlah data tetap.
3. `std::vector` dipakai kalau jumlah data bisa bertambah.

## 06 - Struct dan Class

### Latihan

1. Bedakan `struct` dan `class`.
2. Apa itu constructor?
3. Kapan lebih cocok memakai `class`?

### Jawaban Singkat

1. `struct` biasanya untuk data sederhana, `class` untuk data dan aksi.
2. Constructor adalah fungsi khusus saat benda baru dibuat.
3. Saat data dan tindakannya ingin disatukan.

## 07 - Pointer dan Referensi

### Latihan

1. Apa itu pointer?
2. Apa itu referensi?
3. Kapan referensi lebih nyaman?

### Jawaban Singkat

1. Pointer adalah alamat tempat data berada.
2. Referensi adalah nama lain untuk data yang sama.
3. Saat ingin memakai data tanpa menyalinnya.

## 08 - OOP

### Latihan

1. Apa tujuan OOP?
2. Kenapa bagian alat dipisah menjadi benda-benda kecil?
3. Apa gunanya `virtual` secara sederhana?

### Jawaban Singkat

1. Supaya program lebih rapi dan mudah dirawat.
2. Agar tiap bagian punya tugas jelas.
3. Agar satu benda bisa punya cara kerja berbeda tergantung jenisnya.

## 09 - Template dan Cara Generik

### Latihan

1. Apa itu template?
2. Kenapa template berguna?
3. Kapan sebaiknya tidak memakai template dulu?

### Jawaban Singkat

1. Template adalah pola kode yang bisa dipakai ulang.
2. Supaya satu pola bisa dipakai untuk banyak jenis data.
3. Saat versi biasa masih cukup jelas dan mudah dibaca.

## 10 - C++23 Modern

### Latihan

1. Apa gunanya `std::span`?
2. Kenapa `std::expected` membantu?
3. Apa manfaat `ranges`?

### Jawaban Singkat

1. Untuk melihat potongan data tanpa menyalin.
2. Untuk membawa hasil yang bisa berhasil atau gagal dengan lebih rapi.
3. Untuk mengolah data dengan alur yang lebih enak dibaca.

## 11 - Error dan Debug

### Latihan

1. Apa itu debugging?
2. Kenapa `assert` dipakai?
3. Apa langkah aman saat program gagal?

### Jawaban Singkat

1. Debugging adalah mencari sebab masalah.
2. Untuk mengecek dugaan saat pengembangan.
3. Baca pesan gagal, cari barisnya, lihat perubahan terakhir, lalu kecilkan masalah.

## 12 - Mini Proyek

### Latihan

1. Apa alur mini proyek kipas?
2. Apa yang terjadi saat suhu tinggi?
3. Apa yang harus dilakukan setelah mini proyek berhasil?

### Jawaban Singkat

1. Baca suhu, cek batas, putuskan aksi, tampilkan hasil.
2. Kipas nyala dan status ditampilkan.
3. Ubah sedikit demi sedikit, misalnya batas suhu atau status tambahan.

## 13 - Peta Kode dan Pustaka

### Latihan

1. File mana yang jadi pintu masuk program?
2. File mana yang mengurus layar?
3. File mana yang mengirim data ke spreadsheet?

### Jawaban Singkat

1. `src/main.cpp`
2. `src/Display.cpp`
3. `src/GoogleSheetsClient.cpp`

## 14 - Alur Lengkap Proyek

### Latihan

1. Apa langkah pertama saat alat menyala?
2. Apa yang terjadi saat PIN benar?
3. Data dikirim ke mana saja?

### Jawaban Singkat

1. ESP32 menyala dan setelan dibaca.
2. Pintu dibuka sebentar lalu catatan disimpan.
3. Ke web dan spreadsheet online.

## 15 - Cara Membaca `src` dan `include`

### Latihan

1. Apa bedanya `src/` dan `include/`?
2. Apa yang biasanya ada di `src/App.cpp`?
3. Kenapa `PinMap.h` penting?

### Jawaban Singkat

1. `src/` berisi kerja utama, `include/` berisi bentuk data dan daftar fungsi.
2. Alur kerja utama dan pengendali semua bagian.
3. Supaya angka pin tidak tercecer di banyak tempat.

## Cara Belajar Berikutnya

Kalau kamu ingin lebih kuat, ulangi latihan ini beberapa kali.

Yang penting bukan cepat selesai, tapi makin mudah menjelaskan dengan kata-katamu sendiri.
