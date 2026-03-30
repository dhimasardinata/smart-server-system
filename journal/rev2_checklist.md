# Checklist Kepatuhan Jurnal Rev 2

## A. Format dan Struktur

- [x] Struktur IMRaD lengkap: `INTRODUCTION`, `METHOD`, `RESULT AND DISCUSSION`, `CONCLUSION`
- [x] Abstract tersedia dan dalam rentang 100-250 kata
- [x] Keywords 3-5 kata dan urut alfabet
- [x] Gaya naskah mengikuti pola Arie+Yuniarta (header jurnal, blok Article Info, heading kapital)
- [x] Sitasi author-year konsisten (gaya Arie+Yuniarta)
- [x] Daftar pustaka dikelola dari BibTeX `rev2_references.bib`

## B. Konsistensi Teknis terhadap Kode

- [x] Logika fan dan threshold sesuai `src/App.cpp` + `src/Config.cpp`
- [x] Lockout dan autentikasi sesuai `src/AccessController.cpp`
- [x] Endpoint REST sesuai `src/NetworkServices.cpp` + `README.md`
- [x] Logging ke `telemetry_logs` dan `access_logs` sesuai `src/GoogleSheetsClient.cpp`
- [x] Mode STA/AP fallback + verifikasi internet sesuai `src/WiFiHandler.cpp`

## C. Kepatuhan Alur Pengujian Proposal

- [x] Alur uji mengikuti proposal: unit -> integration -> field before-after -> finalization
- [x] Hasil saat ini dibatasi sebagai implementasi kode + simulasi script test
- [x] Data lapangan before-after ditulis sebagai placeholder, tanpa angka fiktif

## D. Kepatuhan Referensi Proposal

- [x] Referensi hanya dari daftar pustaka Draft Proposal Rev 2
- [x] Tidak ada sumber tambahan di luar proposal
- [x] Seluruh entry `.bib` tersitasi minimal sekali di naskah

## E. Kompilasi

- [x] `pdflatex rev2_jurnal_smart_server.tex`
- [x] `bibtex rev2_jurnal_smart_server`
- [x] `pdflatex rev2_jurnal_smart_server.tex`
- [x] `pdflatex rev2_jurnal_smart_server.tex`
- [x] PDF terbentuk tanpa error fatal

Catatan: kompilasi bersih dari error fatal. Masih ada warning tipografi (`Underfull/Overfull hbox`) karena panjang URL referensi web.
