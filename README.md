# Smart Server System

Proyek ini adalah program untuk ESP32 yang mengatur:

- pembacaan suhu dan kelembapan
- dua kipas pendingin
- akses pintu lewat keypad
- kunci elektrik
- tampilan di layar kecil
- pengiriman data ke spreadsheet online
- dashboard web untuk melihat data dari jauh

## Alur Sederhana

Bayangkan alurnya seperti ini:

```text
ESP32
  -> sensor membaca suhu dan kelembapan
  -> ESP32 memutuskan apakah kipas perlu menyala
  -> keypad dipakai saat orang ingin masuk
  -> relay menyalakan atau mematikan alat
  -> data penting dikirim ke cloud
```

## Gambaran Singkat

Saat alat menyala:

1. ESP32 memulai sistem.
2. Sensor membaca kondisi ruangan.
3. Jika suhu atau kelembapan naik, kipas ikut bekerja.
4. Orang memasukkan PIN lewat keypad.
5. Jika PIN benar, pintu dibuka sebentar.
6. Semua kejadian penting dicatat.
7. Data juga bisa dilihat lewat dashboard web.

## Isi Proyek

- `src/` berisi program utama ESP32
- `include/` berisi bagian-bagian yang dipakai program utama
- `web-dashboard/` berisi tampilan web untuk melihat data
- `google-apps-script/` berisi penerima data untuk spreadsheet
- `tests/` berisi contoh kirim data untuk pengujian

## File Yang Paling Penting

Kalau baru mulai membaca, urutan yang enak:

1. `src/main.cpp`
2. `src/App.cpp`
3. `src/Display.cpp`
4. `src/Config.cpp`
5. `src/NetworkServices.cpp`

## Cara Menjalankan

Build program:

```bash
pio run
```

Upload ke ESP32:

```bash
pio run -t upload
```

Upload lewat jaringan:

```bash
pio run -t upload --environment esp32dev_ota
```

## Halaman Web

Proyek ini juga punya halaman web untuk:

- melihat suhu dan kelembapan
- melihat status kipas
- melihat status pintu
- melihat catatan akses
- mengubah pengaturan
- mengirim pembaruan program lewat jaringan

Foldernya ada di:

- [web-dashboard/README.md](/home/dhimasardinata/Dokumen/server/web-dashboard/README.md)

## Spreadsheet Online

Data yang dikirim ke spreadsheet online ditangani oleh skrip di:

- [google-apps-script/README.md](/home/dhimasardinata/Dokumen/server/google-apps-script/README.md)

## Istilah

Kalau menemui kata yang belum familiar, buka:

- [docs/GLossary.md](/home/dhimasardinata/Dokumen/server/docs/GLossary.md)

## Belajar Dari Nol

Kalau kamu benar-benar baru, mulai dari jalur C++23 ini:

- [docs/tutorial/README.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/README.md)

## Jalur Belajar Awam

Kalau kamu ingin memahami proyek ini tanpa loncat-loncat:

1. baca [docs/tutorial/01-dasar.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/01-dasar.md)
2. lanjut ke [docs/tutorial/02-file-proyek.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/02-file-proyek.md)
3. baca [docs/tutorial/03-esp32-sensor-lcd.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/03-esp32-sensor-lcd.md)
4. lanjut [docs/tutorial/04-jaringan-dan-web.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/04-jaringan-dan-web.md)
5. baca [docs/tutorial/05-spreadsheet-online.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/05-spreadsheet-online.md)
6. terakhir baca [docs/tutorial/06-cara-mengubah-program.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/06-cara-mengubah-program.md)
7. lihat [docs/tutorial/07-alur-lengkap.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/07-alur-lengkap.md)
8. lanjut [docs/tutorial/08-baca-src-include.md](/home/dhimasardinata/Dokumen/server/docs/tutorial/08-baca-src-include.md)

## Catatan Singkat

- pengaturan disimpan di memori internal ESP32
- jika jaringan tidak ada, perangkat bisa membuat jaringan sendiri untuk disetel
- jika file pengaturan rusak, sistem akan mencoba kembali ke bawaan
- pembaruan program bisa dilakukan tanpa kabel USB jika jaringan tersedia
