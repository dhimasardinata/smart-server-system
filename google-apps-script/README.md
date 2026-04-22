# Google Apps Script

Folder ini berisi skrip yang menerima data dari ESP32 lalu menulisnya ke spreadsheet.

Bagian ini adalah jembatan antara ESP32 dan tabel online.

Yang dipakai ada dua lembar data:

- `telemetry_logs`
- `access_logs`

## Apa Bedanya Dua Lembar Ini

- `telemetry_logs` berisi data suhu, kelembapan, kipas, dan status alat
- `access_logs` berisi data orang masuk, PIN, hasil akses, dan alasan

## Cara Kerja

1. ESP32 mengirim data.
2. Skrip menerima data.
3. Skrip menaruh data ke baris spreadsheet yang sesuai.
4. Kalau data tidak cocok, skrip menolak dan memberi pesan gagal.

## Cara Menyiapkan

1. Buat spreadsheet baru.
2. Buka `Extensions` -> `Apps Script`.
3. Ganti isi skrip dengan `Code.gs`.
4. Simpan lalu sebarkan sebagai aplikasi web.
5. Masukkan alamat skrip itu ke pengaturan ESP32.

## File Penting

- `Code.gs` = skrip utama
- `.clasp.json` = catatan sambungan ke proyek Google
- `.claspignore` = daftar file yang tidak perlu ikut dikirim

## Saat Mengubah Isi

- kalau menambah kolom baru, ubah `Code.gs`
- kalau ingin nama lembar data beda, sesuaikan di skrip dan di ESP32
- kalau data tidak masuk, cek apakah alamat skrip masih benar

## Catatan

- skrip ini menerima data dari ESP32
- nama kolom sudah diatur di `Code.gs`
- kalau ada data yang tidak lengkap, skrip akan menolak dan memberi pesan gagal
- kalau spreadsheet belum menerima data, cek izin akses dan alamat skrip
