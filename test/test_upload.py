import requests
import json

GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbxVuisohtU0X2y6SBJhpR7stwr54dERGWv8wgq9KsjWhxZb-eH541N9pq33luIBhrWH4g/exec"

data = {
    "temperature": 25.5,
    "humidity": 65.0
}

print("Sending dummy data to Google Sheets...")
print(f"Temperature: {data['temperature']}°C")
print(f"Humidity: {data['humidity']}%")
print()

try:
    response = requests.post(
        GOOGLE_SCRIPT_URL,
        json=data,
        headers={"Content-Type": "application/json"},
        timeout=30
    )
    print(f"Status Code: {response.status_code}")
    print(f"Response: {response.text}")
    
    if response.status_code == 200:
        print("\n✅ SUCCESS! Data sent to Google Sheets")
    else:
        print("\n❌ FAILED! Check the response above")
        
except Exception as e:
    print(f"\n❌ ERROR: {e}")
