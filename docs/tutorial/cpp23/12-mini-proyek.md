# 12 - Mini Proyek

Bab ini menggabungkan semua yang sudah dipelajari.

## Target

Buat program kecil yang:

- menyimpan suhu
- mengecek apakah terlalu panas
- menyalakan kipas jika perlu
- menampilkan hasil ke layar

## Kerangka Pikir

```text
baca data -> cek batas -> putuskan aksi -> tampilkan hasil
```

## Contoh Sederhana

```cpp
#include <Arduino.h>

// Pin yang dipakai untuk kipas.
const int pinKipas = 5;
// Nilai suhu contoh.
int suhu = 31;

void setup() {
    // Buka jalur tulisan ke Serial Monitor.
    Serial.begin(115200);
    // Siapkan pin sebagai keluaran.
    pinMode(pinKipas, OUTPUT);
}

void loop() {
    // Kalau panas, hidupkan kipas.
    if (suhu > 30) {
        digitalWrite(pinKipas, HIGH);
        Serial.println("Kipas nyala");
    } else {
        // Kalau belum panas, matikan kipas.
        digitalWrite(pinKipas, LOW);
        Serial.println("Kipas mati");
    }

    // Tunggu sebentar sebelum ulang lagi.
    delay(1000);
}
```

## Langkah Lanjut

Setelah mini proyek ini berhasil, coba:

- ubah batas suhu
- tambah kelembapan
- simpan status ke dalam class
- pecah kode jadi beberapa fungsi

## Penutup

Kalau sudah sampai sini, kamu sudah punya bekal yang cukup untuk membaca banyak kode C++ modern dan mulai mengubahnya sendiri.

Kalau mau naik level lagi, ulangi mini proyek ini dengan perubahan kecil:

1. ganti batas suhu
2. tambah pesan saat kipas mulai nyala
3. tambah satu variabel untuk status alat
4. pecah bagian nyala dan mati jadi fungsi sendiri

Itu cara belajar yang paling aman: sedikit demi sedikit, tapi benar-benar paham.
