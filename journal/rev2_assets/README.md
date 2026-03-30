# Placeholder Aset Jurnal Rev 2

Folder ini menampung aset visual untuk naskah `rev2_jurnal_smart_server.tex`.

## Daftar placeholder yang direkomendasikan

1. `fig_architecture_block.png`
- Diagram blok sistem (sensor, ESP32, relay, fan, keypad, solenoid, LCD, cloud).

2. `fig_dataflow_logging.png`
- Alur data dari perangkat ke `telemetry_logs` dan `access_logs`.

3. `fig_state_security.png`
- State keamanan (`READY`, `GRANTED`, `DENIED`, `LOCKOUT`).

4. `tbl_field_test_results.csv`
- Placeholder hasil uji lapangan (belum final).

## Catatan penggunaan

1. Jika file gambar belum tersedia, naskah tetap dapat dikompilasi karena masih memakai tabel dan deskripsi teks.
2. Saat aset final siap, update blok `\includegraphics{...}` pada file `.tex`.
