# 03 - ESP32, Sensor, dan LCD

Bab ini menjelaskan bagian fisik yang dipakai.

## ESP32

ESP32 adalah chip utama yang menjalankan semua perintah.

Kalau dibayangkan, ESP32 itu seperti otak kecil yang bisa:

- membaca sensor
- menyalakan alat
- tersambung ke WiFi
- menampilkan data ke layar

## Sensor

Sensor membaca suhu dan kelembapan.

Di proyek ini, data sensor dipakai untuk:

- menyalakan kipas
- memberi peringatan
- dikirim ke spreadsheet

Sensor ini penting karena keputusan alat bergantung pada hasil bacaan sensor.

## LCD

LCD adalah layar kecil untuk menampilkan kondisi alat.

Di layar bisa muncul:

- suhu
- kelembapan
- status kipas
- status pintu

Layar membantu orang melihat keadaan alat tanpa membuka program.

## Hubungan Antarbagian

Sederhananya:

```text
sensor -> ESP32 -> LCD
```

Sensor membaca data, ESP32 mengolahnya, lalu LCD menampilkan hasilnya.

## Kenapa Jalur Ini Dipakai

Karena kita tidak selalu perlu membuka browser atau komputer.

Kadang cukup lihat layar kecil untuk tahu:

- suhu sekarang berapa
- kelembapan berapa
- pintu terkunci atau tidak

## Contoh Alur Nyata

1. sensor membaca suhu
2. ESP32 melihat apakah suhu lewat batas
3. ESP32 menyalakan kipas
4. LCD menulis hasilnya

## Latihan Kecil

Coba sebutkan:

- bagian mana yang membaca
- bagian mana yang memutuskan
- bagian mana yang menampilkan
