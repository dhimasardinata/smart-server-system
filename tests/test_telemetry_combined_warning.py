"""Skenario: Warning gabungan — suhu dan kelembapan melewati ambang warning."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "telemetry_logs",
    "device_id": "esp32-smart-server-01",
    "temperature_c": 27.6,
    "humidity_pct": 66.5,
    "fan1_on": "true",
    "fan2_on": "true",
    "alarm_state": "NORMAL",
    "door_state": "LOCKED",
    "wifi_rssi": -51,
    "warn_threshold": 27.0,
    "stage2_threshold": 28.0,
    "warn_hum_threshold": 65.0,
    "stage2_hum_threshold": 75.0,
}

resp = requests.get(URL, params=params)
print(f"[WARNING KOMBINASI] Status: {resp.status_code}")
print(resp.text)
