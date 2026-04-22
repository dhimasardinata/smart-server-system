# 07 - Alur Lengkap Proyek

Bab ini menunjukkan alur besar proyek dari awal sampai akhir.

## Alur Utama

Kalau digambar sederhana, alurnya begini:

```text
sensor -> ESP32 -> layar / relay / keypad -> WiFi -> web / spreadsheet
```
## Urutan Kejadian

1. ESP32 menyala.
2. Setelan dibaca dari memori.
3. Sensor suhu dan kelembapan disiapkan.
4. Layar dinyalakan.
5. WiFi dicoba dihubungkan.
6. Kalau WiFi gagal, alat bisa masuk mode penyetelan.
7. Data sensor dibaca berkala.
8. Kipas dan pintu diputuskan berdasarkan kondisi.
9. Data penting dikirim ke web dan spreadsheet.
10. Jika ada akses pintu, catatannya juga disimpan.

## Bagian yang Bekerja Bersama

- `sensor` memberi data
- `ESP32` memutuskan apa yang harus dilakukan
- `relay` menyalakan atau mematikan alat
- `LCD` menampilkan hasil
- `keypad` menerima PIN
- `WiFi` menghubungkan perangkat
- `web` memberi cara lihat dan ubah data
- `spreadsheet` menyimpan catatan jarak jauh

## Alur Saat Orang Memasukkan PIN

```text
tekan tombol -> PIN terkumpul -> dicek -> benar atau salah
```

Kalau benar:

- pintu dibuka sebentar
- status dicatat
- data bisa dikirim ke spreadsheet

Kalau salah:

- akses ditolak
- percobaan salah dihitung
- kalau terlalu banyak gagal, alat dikunci sementara

## Alur Saat Suhu Naik

```text
sensor baca suhu -> ESP32 cek batas -> kipas nyala atau mati
```

Kalau suhu lewat batas:

- kipas pertama bisa ikut menyala
- kipas kedua bisa menyala saat lebih tinggi lagi
- layar menampilkan peringatan
- data dicatat

## Alur Saat Web Dibuka

```text
browser -> ESP32 -> data tampil di halaman web
```

Lewat web, pengguna bisa:

- melihat data terbaru
- mengubah setelan
- melihat catatan
- mengunggah pembaruan program

## Alur Saat Data Dikirim Ke Spreadsheet

```text
ESP32 -> skrip penerima -> spreadsheet
```

Data yang dikirim biasanya berisi:

- waktu
- suhu
- kelembapan
- status kipas
- status pintu
- hasil akses

## Kenapa Alur Ini Penting

Kalau kamu paham alurnya, kamu tidak perlu hafal semua file sekaligus.

Yang penting adalah tahu:

- data datang dari mana
- data diproses di mana
- hasilnya keluar ke mana

## Cara Mengingatnya

Pakai kalimat sederhana ini:

```text
baca -> pikir -> lakukan -> simpan -> tampilkan
```

## Latihan Kecil

Coba tulis ulang alur ini dengan kata-katamu sendiri:

- saat alat menyala
- saat sensor dibaca
- saat PIN dimasukkan
- saat data dikirim
