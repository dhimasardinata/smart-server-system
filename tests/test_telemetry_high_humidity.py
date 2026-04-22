"""Contoh tes: kelembapan sangat tinggi."""
import requests

# Ganti alamat ini kalau skrip penerima data berubah.
URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

# Data contoh yang dikirim ke skrip penerima.
params = {
    "sheet": "telemetry_logs",
    "device_id": "esp32-smart-server-01",
    "temperature_c": 26.0,
    "humidity_pct": 85.0,
    "fan1_on": "true",
    "fan2_on": "true",
    "alarm_state": "NORMAL",
    "door_state": "LOCKED",
    "wifi_rssi": -55,
    "warn_threshold": 27.0,
    "stage2_threshold": 28.0,
    "warn_hum_threshold": 65.0,
    "stage2_hum_threshold": 75.0,
}

# Jalankan permintaan dan tampilkan balasannya.
resp = requests.get(URL, params=params)
print(f"[KELEMBAPAN TINGGI] Status: {resp.status_code}")
print(resp.text)
