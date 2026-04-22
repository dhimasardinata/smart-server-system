# 05 - Teks dan Wadah Data

Bab ini menjelaskan cara menyimpan banyak data.

## Teks

Untuk teks, C++ punya `std::string`.

```cpp
// Teks biasa yang disimpan di memori.
std::string nama = "Andi";
```

## Wadah Data

Wadah data dipakai untuk menyimpan banyak isi.

- `std::array` untuk ukuran tetap
- `std::vector` untuk ukuran yang bisa bertambah

Contoh:

```cpp
#include <array>
#include <vector>

// Data tetap, jumlahnya tidak berubah.
std::array<int, 3> angka_tetap{1, 2, 3};
// Data yang bisa bertambah.
std::vector<int> angka_fleksibel{1, 2, 3};
```

## `std::string_view`

Dipakai kalau hanya ingin melihat teks, bukan memilikinya.

```cpp
void lihat_teks(std::string_view teks) {
    // Kirim isi teks apa adanya.
    Serial.write(reinterpret_cast<const uint8_t*>(teks.data()), teks.size());
    // Pindah baris setelah teks selesai.
    Serial.println();
}
```

## Kapan Dipakai

- `std::string` untuk teks yang akan diubah
- `std::string_view` untuk teks yang hanya dibaca
- `std::array` untuk data yang jumlahnya tetap
- `std::vector` untuk data yang jumlahnya bisa berubah
