# 04 - Jaringan dan Web

Bab ini menjelaskan bagian yang terhubung ke internet dan halaman web.

## Jaringan WiFi

ESP32 bisa tersambung ke WiFi rumah atau kantor.

Kalau belum ada jaringan tersimpan, ESP32 bisa membuat jaringan sendiri untuk penyetelan awal.

Jadi alat ini punya dua cara:

- ikut jaringan yang sudah ada
- membuat jaringan sendiri sementara

## Halaman Web

Halaman web dipakai untuk:

- melihat data langsung
- mengubah pengaturan
- mengirim pembaruan program

Halaman ini dibuka lewat browser, jadi pengguna tidak perlu aplikasi khusus.

## Alur Sederhana

```text
ESP32 -> halaman web -> pengguna
```

Pengguna membuka browser, lalu melihat atau mengubah data.

## Kenapa Ini Penting

Bagian web membuat alat lebih mudah dipakai.

Kalau mau ubah setelan, tidak perlu bongkar perangkat.

Kalau mau lihat data, cukup buka browser di ponsel atau laptop.

## Istilah Sederhana

- `browser` = aplikasi untuk membuka halaman web
- `dashboard` = halaman ringkasan
- `alamat tujuan` = alamat yang dituju saat browser atau ESP32 mengirim data

## Latihan Kecil

Coba jawab:

- kalau WiFi rumah gagal, alat ini melakukan apa
- kalau pengguna membuka browser, apa yang bisa dilakukan
- bagian mana yang dipakai untuk melihat data, bagian mana yang dipakai untuk mengubah data

## Kenapa Ini Penting

Bagian ini memudahkan perangkat dipakai tanpa harus selalu menyambung kabel.
