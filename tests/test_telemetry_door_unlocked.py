"""Skenario: Pintu Terbuka — solenoid aktif, pintu unlocked."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "telemetry_logs",
    "device_id": "esp32-smart-server-01",
    "temperature_c": 25.0,
    "humidity_pct": 55.0,
    "fan1_on": "true",
    "fan2_on": "false",
    "alarm_state": "NORMAL",
    "door_state": "UNLOCKED",
    "wifi_rssi": -50,
    "warn_threshold": 27.0,
    "stage2_threshold": 28.0,
}

resp = requests.get(URL, params=params)
print(f"[PINTU TERBUKA] Status: {resp.status_code}")
print(resp.text)
