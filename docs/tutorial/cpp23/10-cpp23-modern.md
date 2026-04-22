# 10 - C++23 Modern

Bab ini mengenalkan fitur yang lebih baru.

## `std::span`

Dipakai untuk melihat potongan data tanpa menyalinnya.

Bayangkan seperti menunjuk sebagian isi meja, bukan memindahkan seluruh meja.

```cpp
#include <span>

// Lihat isi data tanpa menyalinnya.
void lihat_data(std::span<int> data) {
    // Tulis setiap angka satu per satu.
    for (int x : data) {
        Serial.print(x);
        Serial.print(' ');
    }
    // Pindah baris setelah semua angka ditulis.
    Serial.println();
}
```

## `std::expected`

Dipakai untuk hasil yang bisa berhasil atau gagal tanpa langsung membuat error besar.

Jadi kita bisa mengecek apakah hasilnya aman atau tidak.

Kalau alat terjemah ESP32 yang kamu pakai belum mendukung penuh, cukup pahami dulu idenya.

Contoh pikirnya begini:

```cpp
#include <expected>

// Hasil baca suhu: bisa sukses atau gagal.
std::expected<float, const char*> bacaSuhu(bool sukses) {
    // Kalau gagal, kirim pesan gagal.
    if (!sukses) {
        return std::unexpected("sensor gagal dibaca");
    }

    // Kalau berhasil, kirim angka suhu.
    return 31.5f;
}

// Cek hasil baca suhu.
void cekHasil() {
    // Simpan hasil yang didapat.
    auto hasil = bacaSuhu(true);

    // Kalau berhasil, pakai nilainya.
    if (hasil) {
        Serial.print("Suhu terbaca: ");
        Serial.println(*hasil);
    } else {
        // Kalau gagal, tampilkan alasannya.
        Serial.println(hasil.error());
    }
}
```

## `ranges`

`ranges` membantu memproses data dengan cara yang lebih enak dibaca.

Ibaratnya, ini cara baru untuk menyaring dan mengolah data.

Di ESP32, ini enak dipakai kalau kamu punya banyak data sensor lalu hanya mau ambil yang penting.

Contoh:

```cpp
#include <array>
#include <ranges>

// Daftar suhu contoh.
std::array<int, 5> suhu{28, 29, 31, 33, 30};

// Tampilkan hanya data yang lewat batas.
void lihatYangPanas() {
    // Ambil hanya angka di atas 30.
    for (int nilai : suhu | std::views::filter([](int x) { return x > 30; })) {
        Serial.println(nilai);
    }
}
```

## `if constexpr`

Dipakai kalau keputusan ingin dibuat saat program disiapkan, bukan saat jalan.

Ini berguna kalau ada bagian kode yang hanya cocok untuk tipe tertentu.

Misalnya satu fungsi mau dipakai untuk angka bulat dan angka pecahan, tapi perlakuannya sedikit beda.

## Saran

Fitur modern sangat membantu, tapi pelajari dulu dasarnya supaya tidak bingung.

## Supaya Tidak Tersesat

Fitur baru sering terlihat keren, tapi kalau belum paham dasar, malah bikin bingung.

Urutan yang aman:

1. paham variabel
2. paham keputusan
3. paham fungsi
4. paham data yang dikelompokkan
5. baru masuk ke fitur baru

## Latihan Kecil

Lihat satu fitur modern di bab ini, lalu jawab dengan kata-katamu sendiri:

- dipakai untuk apa
- masalah apa yang diselesaikan
- kapan sebaiknya tidak dipakai dulu
