"""Skenario: Alarm — suhu melebihi stage2 threshold, alarm aktif."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "telemetry_logs",
    "device_id": "esp32-smart-server-01",
    "temperature_c": 29.8,
    "humidity_pct": 70.0,
    "fan1_on": "true",
    "fan2_on": "true",
    "alarm_state": "ALARM",
    "door_state": "LOCKED",
    "wifi_rssi": -55,
    "warn_threshold": 27.0,
    "stage2_threshold": 28.0,
}

resp = requests.get(URL, params=params)
print(f"[ALARM] Status: {resp.status_code}")
print(resp.text)
