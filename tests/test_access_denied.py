"""Skenario: PIN Salah — percobaan pertama gagal."""
import requests

URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

params = {
    "sheet": "access_logs",
    "device_id": "esp32-smart-server-01",
    "user_id": "unknown",
    "display_name": "Unknown",
    "result": "DENIED",
    "reason": "INVALID_PIN",
    "failed_count": 1,
    "lockout_until": 0,
    "door_state": "LOCKED",
}

resp = requests.get(URL, params=params)
print(f"[PIN SALAH] Status: {resp.status_code}")
print(resp.text)
