"""Skenario: Akses Diterima — PIN benar, pintu terbuka."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "access_logs",
    "device_id": "esp32-smart-server-01",
    "user_id": "admin",
    "display_name": "Administrator",
    "result": "GRANTED",
    "reason": "PIN_MATCH",
    "failed_count": 0,
    "lockout_until": 0,
    "door_state": "UNLOCKED",
}

resp = requests.get(URL, params=params)
print(f"[AKSES DITERIMA] Status: {resp.status_code}")
print(resp.text)
