# 04 - Fungsi dan Ruang Kerja

Bab ini menjelaskan cara memecah program menjadi bagian kecil.

## Fungsi

Fungsi adalah kumpulan langkah yang bisa dipanggil ulang.

```cpp
// Fungsi ini menjumlahkan dua angka.
int jumlahkan(int a, int b) {
    // Kembalikan hasil penjumlahan.
    return a + b;
}
```

## Kenapa Fungsi Penting

- kode jadi tidak panjang
- lebih mudah dibaca
- lebih mudah diuji

## Ruang Kerja

Variabel yang dibuat di dalam fungsi biasanya hanya hidup di fungsi itu.

```cpp
void contoh() {
    // Variabel ini hanya hidup di dalam fungsi.
    int angka = 10;
}
```

`angka` tidak bisa dipakai di luar fungsi `contoh()`.

## Parameter

Parameter adalah data yang masuk ke fungsi.

```cpp
// Nama penerima pesan.
void sapa(const char* nama) {
    // Tulis sapaan ke Serial Monitor.
    Serial.print("Halo, ");
    Serial.println(nama);
}

// Nyalakan pin yang dipakai kipas atau lampu.
void nyalakanKipas(int pin) {
    // Kirim sinyal nyala ke pin.
    digitalWrite(pin, HIGH);
}

void setup() {
    // Buka jalur tulisan.
    Serial.begin(115200);
    // Contoh memanggil fungsi.
    sapa("Andi");
}
```

## Saran

Kalau satu potong kode terasa terlalu panjang, pecah jadi fungsi kecil.

## Contoh Nyata di ESP32

Bayangkan kamu punya tugas seperti ini:

- nyalakan kipas
- matikan kipas
- kirim pesan ke layar
- baca sensor

Kalau semua ditulis di satu tempat, kode cepat penuh.
Kalau dipisah jadi fungsi kecil, tiap bagian lebih mudah dipahami.

## Latihan Kecil

Buat tiga fungsi kecil:

1. satu untuk menyalakan lampu
2. satu untuk mematikan lampu
3. satu untuk menulis pesan ke Serial Monitor

Tujuannya bukan agar cepat selesai, tapi agar kamu terbiasa memecah kerja besar jadi potongan kecil.
