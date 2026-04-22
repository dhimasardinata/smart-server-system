# 01 - Dasar dan Cara Berpikir

Bab ini menjawab: "C++ itu apa?"

## C++ Itu Apa

C++ adalah bahasa untuk memberi perintah ke komputer.

Komputer tidak menebak maksud kita, jadi perintah harus jelas dan urut.

Ada juga alat penerjemah yang mengubah tulisan C++ menjadi bentuk yang bisa dijalankan komputer. Di tutorial ini, alat itu disebut `compiler`.

## Cara Berpikir Program

Sederhananya:

1. baca data
2. olah data
3. keluarkan hasil

Kalau dipakai di ESP32, alurnya sering begini:

1. baca sensor suhu
2. cek apakah nilainya tinggi
3. nyalakan kipas atau relay
4. kirim keterangan ke Serial Monitor

Contoh:

```cpp
#include <Arduino.h>

// Pada ESP32, teks biasanya dikirim ke Serial Monitor.
void setup() {
    // Buka jalur tulisan ke komputer.
    Serial.begin(115200);
    // Beri tahu bahwa alat sudah siap.
    Serial.println("ESP32 siap");
}

void loop() {
    // Di contoh paling dasar, loop boleh kosong dulu.
}
```

## Penjelasan Singkat

- `#include <Arduino.h>`: ambil alat bawaan ESP32
- `setup()`: bagian yang dijalankan satu kali saat ESP32 menyala
- `loop()`: bagian yang diulang terus selama ESP32 hidup
- `Serial.begin(...)`: menyiapkan jalur untuk mengirim teks ke laptop
- `Serial.println(...)`: menulis teks agar kelihatan di Serial Monitor
- `compiler`: alat yang membaca kode C++ lalu menyiapkannya jadi program yang bisa dijalankan

## Catatan Penting

- C++ dibaca dari atas ke bawah
- satu tanda kecil bisa membuat program gagal
- kebiasaan baik paling awal adalah menulis kode rapi

## Kenapa Bab Ini Penting

Kalau kamu paham alur, kamu tidak mudah panik saat melihat kode yang panjang.

Yang dicari dulu bukan semua detailnya, tapi urutannya:

1. apa yang masuk
2. apa yang diproses
3. apa yang keluar

## Latihan Kecil

Coba ubah tulisan `ESP32 siap` menjadi pesan lain yang kamu mau.

Lalu pikirkan:

- kapan pesan itu muncul
- bagian mana yang dijalankan sekali
- bagian mana yang diulang terus

## Yang Perlu Diingat

Kalau ingin mengerti program, jangan langsung lihat detail kecil dulu.
Lihat dulu alurnya:

```text
input -> proses -> output
```
