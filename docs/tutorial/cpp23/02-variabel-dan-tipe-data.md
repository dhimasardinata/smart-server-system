# 02 - Variabel dan Tipe Data

Bab ini menjelaskan tempat menyimpan data.

## Variabel

Variabel adalah kotak kecil untuk menyimpan nilai.

Bayangkan variabel seperti label tempel di kotak. Labelnya punya nama, kotaknya punya isi.

```cpp
// Angka bulat untuk suhu.
int suhu = 27;
// Jawaban ya atau tidak untuk status kipas.
bool kipas = true;
```

## Tipe Data

Tipe data memberi tahu isi kotak itu seperti apa.

- `int` untuk bilangan bulat
- `float` atau `double` untuk angka pecahan
- `bool` untuk benar atau salah
- `char` untuk satu huruf
- `std::string` untuk teks

## `const` dan `constexpr`

- `const` berarti nilainya tidak boleh diubah
- `constexpr` berarti nilainya bisa dihitung saat program disiapkan

Contoh:

```cpp
// Nilai ini tidak boleh berubah.
const int batas = 30;
// Nilai ini bisa dihitung sejak program disiapkan.
constexpr int jumlah_kaki_laba_laba = 8;
```

## `auto`

`auto` membuat alat penerjemah C++ menebak tipe data dari isi kanan.

```cpp
// C++ menebak tipe data dari isi kanan.
auto nama = "Budi";
```

## Saran Praktis

Kalau baru mulai:

- pakai tipe yang jelas dulu
- jangan terlalu sering pakai `auto` kalau belum paham isinya
- tulis nama variabel yang mudah dimengerti

## Contoh Paling Sederhana

```cpp
// Contoh data sederhana yang sering dipakai di ESP32.
int umur = 20;
// Angka pecahan dipakai kalau perlu nilai dengan koma.
float tinggi = 170.5f;
// Nilai benar atau salah dipakai untuk status.
bool aktif = true;
```

- `int` dipakai untuk angka biasa tanpa koma
- `float` dipakai untuk angka yang punya koma
- `bool` dipakai untuk jawaban ya atau tidak

## Contoh Nyata di ESP32

```cpp
// Suhu dari sensor.
int suhu = 31;
// Status kipas.
bool kipasMenyala = false;
// Pin relay yang dipakai untuk menyalakan kipas.
const int pinRelay = 5;
```

Di sini:

- `suhu` menyimpan hasil baca sensor
- `kipasMenyala` menyimpan keadaan alat
- `pinRelay` menyimpan kaki yang dipakai untuk menyalakan alat

## Cara Memilih Tipe

Pilih tipe yang paling sederhana dulu.

- kalau angka bulat, pakai `int`
- kalau angka ada koma, pakai `float`
- kalau cuma dua pilihan, pakai `bool`
- kalau teks, pakai teks

## Latihan Kecil

Buat tiga variabel untuk:

1. suhu
2. kelembapan
3. status kipas

Lalu pikirkan, mana yang berisi angka, mana yang berisi ya/tidak.
