# 09 - Template dan Cara Generik

Bab ini menjelaskan cara menulis kode yang bisa dipakai untuk banyak tipe data.

## Template

Template adalah pola kode yang bisa dipakai ulang.

Sederhananya, template itu seperti resep kosong yang bisa diisi dengan bahan berbeda.

```cpp
// Pola fungsi untuk banyak tipe data.
template <typename T>
T tambah(T a, T b) {
    // Kembalikan hasil penjumlahan.
    return a + b;
}
```

## Kenapa Berguna

Daripada menulis fungsi yang sama berkali-kali, kita bisa membuat satu pola.

## Contoh Pikir Sederhana

```cpp
// Wadah sederhana yang bisa menyimpan tipe apa saja.
template <typename T>
class Kotak {
public:
    // Simpan nilai ke dalam kotak.
    void simpan(T nilai) { data = nilai; }
    // Ambil nilai dari kotak.
    T ambil() const { return data; }

private:
    T data{};
};
```

## `concept`

Di C++23, `concept` dipakai untuk memberi batasan pada jenis data yang boleh masuk.

Kalau dibuat lebih awam, `concept` itu seperti pintu masuk: hanya data yang cocok yang boleh lewat.

Contoh sederhana:

```cpp
#include <concepts>
#include <type_traits>

// Batas: hanya tipe angka yang boleh masuk.
template <typename T>
concept Angka = std::is_arithmetic_v<T>;

// Batasi nilai supaya tidak lewat batas bawah atau atas.
template <Angka T>
T batasi(T nilai, T bawah, T atas) {
    // Kalau terlalu kecil, naikkan ke batas bawah.
    if (nilai < bawah) return bawah;
    // Kalau terlalu besar, turunkan ke batas atas.
    if (nilai > atas) return atas;
    // Kalau sudah pas, kembalikan apa adanya.
    return nilai;
}
```

## Saran

- pakai template kalau memang butuh
- jangan dipakai terlalu awal kalau tujuanmu masih belajar dasar

## Kapan Perlu Dipakai

Pakai pola seperti ini kalau kamu punya pekerjaan yang sama, tapi untuk jenis data yang berbeda.

Contohnya:

- fungsi untuk angka bulat dan angka pecahan
- wadah data untuk suhu, kelembapan, atau tegangan
- alat bantu yang mau dipakai ulang di banyak tempat

## Kapan Tidak Perlu Dipakai

Kalau kamu masih bisa menulis versi biasa dengan jelas, sering kali itu lebih mudah dibaca.

## Latihan Kecil

Coba pikirkan satu fungsi yang bisa dipakai untuk:

1. angka suhu
2. angka kelembapan
3. angka tegangan

Semua masih sama-sama angka, jadi pola yang sama bisa dipakai ulang.
