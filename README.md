# Smart Server System (ESP32)

Firmware ESP32 untuk:

- Monitoring suhu & kelembapan (SHT21)
- Kontrol pendinginan otomatis 2 fan (relay)
- Keamanan akses keypad 4x4 + solenoid lock
- Lockout keamanan (3x gagal, default 120 detik)
- Logging cloud ke Google Sheets (`telemetry_logs`, `access_logs`)
- Dashboard lokal ESP32 dan dashboard cloud (folder `web-dashboard`)

## Pin Mapping

- I2C SDA/SCL: `GPIO 21/22`
- Keypad Row: `32, 33, 25, 26`
- Keypad Col: `27, 14, 12, 13`
- Relay Fan1/Fan2/Solenoid: `4, 18, 19`

## Build

```bash
pio run
```

## Upload

```bash
pio run -t upload
```

## Endpoint Lokal

- `GET /`
- `GET /setup`
- `GET /api/state`
- `GET/POST /api/config/thermal`
- `GET/POST /api/config/security`
- `GET/POST /api/users`
- `DELETE /api/users/{userId}`
- `POST /api/send`
- `GET /api/wifi/scan`
- `POST /api/wifi/connect`

## Catatan

- HTTPS Google Apps Script saat ini menggunakan mode `setInsecure`.
- Status pintu diturunkan dari event akses/solenoid (tanpa reed switch).
- Script backend Google Apps Script tersedia di `google-apps-script/Code.gs`.
- Tidak ada endpoint legacy (`/api/data` dan `/api/config`) serta tidak ada fallback schema lama.
