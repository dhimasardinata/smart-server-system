# Web Dashboard

Folder ini berisi halaman web untuk melihat data dari Spreadsheet online.

Halaman ini dipakai saat kamu ingin melihat data lewat browser, tanpa buka file program.

Yang bisa dilihat di sini:

- suhu
- kelembapan
- status kipas
- status pintu
- catatan akses
- ambang batas yang dipakai ESP32
- jaringan yang tersimpan
- pengaturan alat

## Bagian File

- `index.html` = susunan halaman
- `styles.css` = warna, jarak, dan tampilan
- `app.js` = perilaku halaman

## Cara Kerja

1. Browser membuka halaman.
2. Halaman mengambil data dari ESP32.
3. Data ditampilkan di layar.
4. Pengguna bisa mengubah setelan.
5. Perubahan dikirim kembali ke ESP32.

## Cara Pakai

1. Buka `index.html` di browser.
2. Tunggu data muncul.
3. Ubah setelan jika perlu.
4. Simpan perubahan.
5. Bila ada upload firmware, pilih file `.bin` terlebih dahulu.

## Yang Perlu Dipahami

- halaman ini tidak berdiri sendiri, tetapi berbicara dengan ESP32
- kalau data tidak muncul, biasanya masalah ada di jaringan atau alamat tujuan
- kalau tampilan berantakan, lihat `styles.css`
- kalau tombol tidak bekerja, lihat `app.js`

## Catatan

- halaman ini membaca data dari Spreadsheet online
- file utama ada di `index.html`
- gaya tampilan ada di `styles.css`
- perilaku halaman ada di `app.js`
- jika browser gagal memuat, cek dulu koneksi ke ESP32
