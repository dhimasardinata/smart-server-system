# Smart Server Cloud Dashboard

Dashboard web untuk monitoring cloud dari Google Sheets dengan 2 worksheet:

- `telemetry_logs`
- `access_logs`

## Struktur kolom

### `telemetry_logs`
`timestamp, device_id, temperature_c, humidity_pct, fan1_on, fan2_on, alarm_state, door_state, wifi_rssi`

### `access_logs`
`timestamp, device_id, user_id, display_name, result, reason, failed_count, lockout_until, door_state`

## Deploy

1. Pastikan spreadsheet sudah public read.
2. Deploy folder `web-dashboard` ke Netlify.
3. Masukkan Spreadsheet ID pada dashboard.
