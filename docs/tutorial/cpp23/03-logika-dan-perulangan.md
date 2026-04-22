# 03 - Logika dan Perulangan

Bab ini menjelaskan cara program mengambil keputusan.

## `if`

```cpp
#include <Arduino.h>

// Pin relay untuk kipas.
int pinKipas = 5;
// Nilai suhu sementara.
int suhu = 31;

void setup() {
    // Buka jalur tulisan ke Serial Monitor.
    Serial.begin(115200);
    // Siapkan pin sebagai keluaran.
    pinMode(pinKipas, OUTPUT);
}

void cekSuhu() {
    // Di program nyata, pin biasanya disiapkan dulu di setup().
    if (suhu > 30) {
        // Kalau panas, nyalakan kipas.
        digitalWrite(pinKipas, HIGH);
        // Beri tahu pengguna.
        Serial.println("Kipas dinyalakan");
    } else {
        // Kalau belum panas, matikan kipas.
        digitalWrite(pinKipas, LOW);
        // Beri tahu pengguna.
        Serial.println("Kipas dimatikan");
    }
}

void loop() {
    // Cek suhu berkali-kali.
    cekSuhu();
    // Tunggu sebentar sebelum cek lagi.
    delay(1000);
}
```

## `switch`

Dipakai kalau pilihan ada beberapa dan jelas.

```cpp
void lihatMode(int mode) {
    switch (mode) {
        // Mode 1 berarti alat siap.
        case 1: Serial.println("Siap"); break;
        // Mode 2 berarti alat aktif.
        case 2: Serial.println("Aktif"); break;
        // Kalau bukan dua itu, tampilkan pesan umum.
        default: Serial.println("Tidak dikenal"); break;
    }
}
```

## Perulangan

### `for`

Dipakai kalau jumlah ulangannya jelas.

```cpp
for (int i = 0; i < 3; ++i) {
    // Tampilkan nomor percobaan.
    Serial.print("Coba ke-");
    Serial.println(i + 1);
    // Beri jeda singkat supaya tulisan mudah dibaca.
    delay(200);
}
```

### `while`

Dipakai kalau ulangannya berhenti saat syarat berubah.

```cpp
while (suhu < 30) {
    // Naikkan suhu sedikit demi sedikit dalam contoh.
    ++suhu;
    // Tampilkan perubahan suhu.
    Serial.print("Suhu naik jadi ");
    Serial.println(suhu);
}
```

## Saran

- pakai `if` untuk keputusan sederhana
- pakai `switch` untuk banyak pilihan yang tetap
- pakai `for` kalau jumlah putaran sudah jelas

## Kenapa Ini Dipakai Terus

Di ESP32, program sering melakukan hal yang sama berulang:

- cek sensor
- tentukan tindakan
- ulangi lagi

Itulah kenapa `if`, `switch`, `for`, dan `while` sering muncul di kode nyata.

## Latihan Kecil

Coba bayangkan alat ini:

- kalau suhu di atas 30, kipas nyala
- kalau suhu di bawah atau sama dengan 30, kipas mati

Tulis ulang dengan kata-katamu sendiri sebelum mencoba menulis kodenya.
