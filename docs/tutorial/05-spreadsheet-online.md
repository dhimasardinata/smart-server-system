# 05 - Spreadsheet Online

Bab ini menjelaskan penyimpanan data di internet.

## Apa yang Disimpan

Data yang dikirim biasanya berupa:

- suhu
- kelembapan
- status kipas
- status pintu
- hasil akses
- alasan akses

Data ini dipakai supaya catatan tidak hilang di alat saja.

## Alur Sederhana

```text
ESP32 -> skrip penerima -> spreadsheet online
```

Artinya:

- ESP32 menyiapkan data
- skrip penerima menerima data
- spreadsheet menyimpan data dalam bentuk tabel

## Kenapa Dipakai

Supaya data tidak cuma terlihat di alat, tetapi juga bisa dicek dari mana saja.

Ini berguna kalau kamu ingin:

- lihat riwayat
- cek kejadian lama
- bikin catatan harian
- lihat pola naik turunnya suhu

## Hasil Akhir

Data yang tersimpan bisa dipakai untuk:

- melihat riwayat
- membuat grafik
- mencari kejadian lama

## Kenapa Pakai Spreadsheet

Karena mudah dibuka dan mudah dipahami.

Orang awam biasanya lebih cepat paham tabel daripada file mentah.

## Latihan Kecil

Coba bayangkan data ini:

- waktu
- suhu
- kelembapan
- status pintu

Kalau masuk spreadsheet, kolom mana yang paling cocok untuk masing-masing data?
