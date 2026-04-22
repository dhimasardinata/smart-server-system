# 11 - Error dan Debug

Bab ini membantu kalau program tidak jalan.

## Tanda Salah

Kalau program gagal, biasanya ada:

- pesan error
- baris yang ditunjuk
- perilaku yang tidak sesuai

## Cara Mencari Masalah

1. baca pesan paling atas
2. cari baris yang ditunjuk
3. lihat perubahan terakhir
4. kecilkan masalah jadi bagian paling sederhana

## `assert`

`assert` dipakai untuk mengecek dugaan saat pengembangan.

Kalau hasil cek tidak sesuai, program akan berhenti dan memberi tanda bahwa ada yang salah.

```cpp
#include <cassert>
// Pastikan angka lebih besar dari nol saat dikembangkan.
assert(angka > 0);
```

Kalau ini gagal, artinya nilai `angka` tidak sesuai dengan harapan kita.

## Debugging

Debugging berarti mencari sebab masalah satu per satu.

Kuncinya:

- jangan panik
- jangan ubah banyak hal sekaligus
- ulangi sampai ketemu sumbernya

Di ESP32, `Serial.println(...)` biasanya dipakai untuk menaruh petunjuk kecil di tengah jalan supaya kita tahu program sudah sampai di bagian mana.

## Saran

Kalau bingung, coba buat contoh paling kecil yang masih gagal. Dari situ masalah biasanya lebih mudah kelihatan.
