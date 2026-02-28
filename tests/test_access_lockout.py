"""Skenario: Lockout — terlalu banyak percobaan gagal, terminal terkunci."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "access_logs",
    "device_id": "esp32-smart-server-01",
    "user_id": "unknown",
    "display_name": "Unknown",
    "result": "LOCKOUT",
    "reason": "MAX_ATTEMPTS_EXCEEDED",
    "failed_count": 3,
    "lockout_until": 120,
    "door_state": "LOCKED",
}

resp = requests.get(URL, params=params)
print(f"[LOCKOUT] Status: {resp.status_code}")
print(resp.text)
