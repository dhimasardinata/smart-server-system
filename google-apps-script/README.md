# Google Apps Script Setup

Use this script to receive telemetry/access logs from ESP32 and append to two sheets:

- `telemetry_logs`
- `access_logs`

## 1) Prepare Spreadsheet

1. Create a new Google Spreadsheet.
2. (Optional) Create sheets manually named:
   - `telemetry_logs`
   - `access_logs`
3. Open the spreadsheet URL.

## 2) Create Apps Script

1. In Spreadsheet: `Extensions` -> `Apps Script`.
2. Replace default code with content from `Code.gs`.
3. Save project.

## 3) Deploy Web App

1. `Deploy` -> `New deployment`.
2. Type: `Web app`.
3. Execute as: `Me`.
4. Who has access: `Anyone`.
5. Click `Deploy`.
6. Copy the Web App URL.

## 4) Configure ESP32

Set the Web App URL in local setup page (`/setup`):

- `googleScriptUrl`

Firmware will send:

- Telemetry: `sheet=telemetry_logs`
- Access: `sheet=access_logs`

Strict mode:

- `sheet` is mandatory and must be valid.
- Canonical snake_case fields only.
- Missing/invalid field -> request rejected (`ok:false`).

## 5) Test quickly in browser

Replace `<WEB_APP_URL>` with your deployment URL:

```text
<WEB_APP_URL>?sheet=telemetry_logs&timestamp=2026-03-01T10:00:00&device_id=esp32-smart-server-01&temperature_c=28.1&humidity_pct=64.3&fan1_on=true&fan2_on=false&alarm_state=NORMAL&door_state=LOCKED&wifi_rssi=-54
```

```text
<WEB_APP_URL>?sheet=access_logs&timestamp=2026-03-01T10:00:00&device_id=esp32-smart-server-01&user_id=admin&display_name=Administrator&result=GRANTED&reason=VALID_PIN&failed_count=0&lockout_until=0&door_state=UNLOCKING
```
