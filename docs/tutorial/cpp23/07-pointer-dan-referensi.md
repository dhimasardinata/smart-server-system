# 07 - Pointer dan Referensi

Bab ini bagian yang sering bikin bingung, jadi pelan-pelan.

## Pointer

Pointer adalah alamat tempat data berada.

```cpp
// Nilai asli.
int angka = 10;
// Simpan alamat dari angka.
int* alamat = &angka;
```

`&angka` berarti "ambil alamat dari `angka`".

## Referensi

Referensi adalah nama lain untuk data yang sama.

```cpp
// Nilai asli.
int angka = 10;
// Nama lain untuk nilai yang sama.
int& r = angka;
```

Contoh yang lebih dekat ke ESP32:

```cpp
// Wadah kecil untuk hasil sensor.
struct Bacaan {
    // Nilai suhu.
    float suhu;
    // Nilai kelembapan.
    float kelembapan;
};

// Ubah data melalui referensi.
void perbarui(Bacaan& data) {
    // Isi suhu dengan contoh nilai baru.
    data.suhu = 31.5f;
}

// Lihat data melalui pointer.
void tampilkan(const Bacaan* data) {
    // Kalau kosong, jangan lanjut.
    if (data == nullptr) {
        return;
    }

    // Cetak suhu dari data yang ditunjuk.
    Serial.print("Suhu: ");
    Serial.println(data->suhu);
}
```

## Bedanya Singkat

- pointer menyimpan alamat
- referensi seperti nama panggilan untuk data yang sama

## Kenapa Penting

Di program besar, kita sering ingin:

- tidak menyalin data besar berulang-ulang
- mengubah data asli
- mengirim data dengan lebih hemat

## Tips Aman

- kalau belum perlu pointer, jangan dipakai dulu
- kalau bisa pakai referensi, biasanya lebih nyaman dibaca
