# AI Context Strict Rules - Smart Server Journal Rev 2

Dokumen ini adalah aturan ketat untuk setiap revisi jurnal berikutnya agar konteks teknis tidak berubah dan tidak keluar dari proposal.

## 1) Scope Tetap

1. Naskah hanya membahas Smart Server ESP32 untuk monitoring suhu-kelembapan dan keamanan akses rak server di CV Gundara Solusi Bersama, Ungaran.
2. Fokus implementasi tetap pada sistem yang ada di repositori ini, bukan rancangan baru.
3. Tidak menambahkan klaim fitur yang tidak ada pada kode saat ini.

## 2) Aturan Format Jurnal (Non-Negotiable)

1. Struktur wajib IMRaD:
- INTRODUCTION
- METHOD
- RESULT AND DISCUSSION
- CONCLUSION
2. Bahasa naskah: Bahasa Indonesia.
3. Abstract wajib 100-250 kata.
4. Keywords wajib 3-5 kata dan urut alfabet.
5. Gaya penyajian mengikuti pola Arie+Yuniarta (header jurnal, blok Article Info, abstract, keywords, heading kapital).
6. Sitasi wajib author-year (gaya Arie+Yuniarta), bukan numerik IEEE.
7. Daftar pustaka wajib dikelola dari file BibTeX (`rev2_references.bib`).
8. Dilarang memasukkan bagian administratif proposal kampus.

## 3) Fakta Teknis Yang Tidak Boleh Diubah

1. Sensor lingkungan: SHT21.
2. Default thermal config:
- `warnThresholdC = 27.0`
- `stage2ThresholdC = 28.0`
- `fan1BaselineOn = true`
3. Logika fan wajib sama:
- `warning = valid && temperature > warnThresholdC`
- `fan2On = valid && temperature >= stage2ThresholdC`
- `fan1On = fan1BaselineOn || warning || fan2On`
4. Default keamanan wajib sama:
- `maxFailedAttempts = 3`
- `keypadLockoutSec = 120`
- `solenoidUnlockSec = 10`
5. Endpoint REST aktif wajib sama:
- `GET /api/state`
- `GET/POST /api/config/thermal`
- `GET/POST /api/config/security`
- `GET/POST /api/users`
- `DELETE /api/users/{userId}`
- `POST /api/send`
- `GET /api/wifi/scan`
- `POST /api/wifi/connect`
6. Logging cloud:
- telemetry ke `telemetry_logs`
- access ke `access_logs`
7. Konektivitas:
- STA mode sebagai jalur utama
- fallback AP mode jika koneksi gagal
- verifikasi internet sebelum operasi normal
8. Keterbatasan wajib ditulis:
- belum memakai reed switch fisik
- HTTPS client masih `setInsecure`

## 4) Aturan Klaim Ilmiah

1. Dilarang mengklaim uji lapangan selesai jika data lapangan belum tersedia.
2. Semua hasil wajib diberi label:
- `hasil implementasi kode`
- `hasil simulasi script test`
- `placeholder rencana uji lapangan`
3. Dilarang membuat angka performa fiktif.
4. Alur pengujian wajib mengikuti proposal: unit -> integrasi -> field before-after -> finalisasi.

## 5) Aturan Sitasi

1. Referensi hanya dari daftar pustaka Draft Proposal REV 2.
2. Tidak menambah sumber baru di luar daftar proposal.
3. Tidak boleh ada referensi fiktif.
4. Semua entry `.bib` harus tersitasi minimal sekali di naskah.

## 6) Aturan Sinkronisasi Konteks

1. Sebelum revisi jurnal, wajib cek ulang file sumber berikut:
- `README.md`
- `src/App.cpp`
- `src/AccessController.cpp`
- `src/NetworkServices.cpp`
- `src/GoogleSheetsClient.cpp`
- `src/WiFiHandler.cpp`
- `src/Config.cpp`
- `tests/`
2. Jika narasi bertentangan dengan kode terbaru, gunakan kode sebagai sumber kebenaran teknis.
3. Semua revisi wajib lolos `journal/rev2_checklist.md`.
