# 01 - Dasar

Bab ini menjelaskan hal paling awal.

## Apa itu Program

Program adalah kumpulan perintah yang dijalankan alat.

Di proyek ini:

- ESP32 membaca kondisi
- ESP32 memutuskan tindakan
- ESP32 menyalakan atau mematikan alat

Kalau dijelaskan sangat sederhana, program itu seperti daftar instruksi:

1. baca keadaan
2. pikirkan langkah berikutnya
3. lakukan aksi

## Contoh Sederhana

Bayangkan alat perlu melakukan ini:

- kalau ruangan panas, kipas nyala
- kalau ruangan normal, kipas mati

Itu sudah cukup disebut program, walau bentuknya masih sangat kecil.

## Apa itu File

File adalah tempat menyimpan isi tertentu.

Contohnya:

- `.cpp` berisi bagian kerja utama
- `.h` berisi daftar bagian yang dipakai bersama
- `.html` berisi tampilan halaman
- `.css` berisi gaya tampilan
- `.js` berisi perilaku halaman
- `.py` berisi skrip contoh pengujian

Setiap file punya tugas sendiri, supaya program besar tetap rapi.

## Kenapa File Dipisah

Kalau semua ditaruh di satu file, program jadi susah dibaca.

Dengan dipisah:

- bagian layar bisa diurus di satu tempat
- bagian sensor di tempat lain
- bagian web di tempat lain lagi

Jadi kalau ada masalah, kita lebih mudah mencari sumbernya.

## Cara Membaca Proyek Ini

Kalau baru mulai, baca dari yang paling umum dulu:

1. [README.md](/home/dhimasardinata/Dokumen/server/README.md)
2. [docs/GLossary.md](/home/dhimasardinata/Dokumen/server/docs/GLossary.md)
3. `src/main.cpp`
4. `src/App.cpp`

Kalau sudah agak paham, lanjutkan ke:

5. `src/Config.cpp`
6. `src/Display.cpp`
7. `src/Sensors.cpp`
8. `src/WiFiHandler.cpp`

## Istilah Penting

- `ESP32` = otak alat
- `sensor` = alat baca
- `relay` = saklar
- `LCD` = layar kecil

## Cara Belajar Yang Aman

Jangan langsung baca semua file.

Urutan yang aman:

1. paham fungsi alatnya
2. paham file mana mengurus apa
3. paham alur data
4. baru masuk ke detail kode

## Latihan Kecil

Coba jawab dengan kata-katamu sendiri:

- alat ini dipakai untuk apa
- data apa yang dibaca
- data apa yang ditampilkan
