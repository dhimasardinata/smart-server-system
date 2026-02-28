import os
import urllib.request
import zipfile
import shutil
import json
from os.path import join, exists

Import("env")

PROJECT_DIR = env.subst("$PROJECT_DIR")
TOOLS_DIR = join(PROJECT_DIR, "tools")
ESPTOOL_DIR = join(TOOLS_DIR, "esptoolspy")
ZIP_PATH = join(TOOLS_DIR, "esptool.zip")
URL = "https://github.com/espressif/esptool/archive/refs/tags/v5.1.0.zip"

PACKAGE_JSON = {
    "name": "tool-esptoolpy",
    "version": "5.1.0",
    "description": "ESP32/ESP8266 ROM Bootloader utility (custom)",
    "keywords": ["esptool", "espressif", "esp32", "flash"],
    "homepage": "https://github.com/espressif/esptool",
    "license": "GPL-2.0-or-later",
    "system": "*",
    "repository": {
        "type": "git",
        "url": "https://github.com/espressif/esptool.git"
    }
}

def generate_package_json():
    package_path = join(ESPTOOL_DIR, "package.json")
    if not exists(package_path):
        with open(package_path, "w") as f:
            json.dump(PACKAGE_JSON, f, indent=2)
        print(f"[TempMonitor] Generated esptool package.json")

def setup_esptool():
    if not exists(TOOLS_DIR):
        os.makedirs(TOOLS_DIR)
    if not exists(ESPTOOL_DIR):
        if not exists(ZIP_PATH):
            print("[TempMonitor] Downloading ESPTool...")
            urllib.request.urlretrieve(URL, ZIP_PATH)
        print("[TempMonitor] Extracting ESPTool...")
        with zipfile.ZipFile(ZIP_PATH, "r") as zip_ref:
            extract_temp = join(TOOLS_DIR, "temp_esptool")
            zip_ref.extractall(extract_temp)
            extracted_name = os.listdir(extract_temp)[0]
            shutil.move(join(extract_temp, extracted_name), ESPTOOL_DIR)
            shutil.rmtree(extract_temp)
    esptool_py = join(ESPTOOL_DIR, "esptool.py")
    if exists(esptool_py):
        generate_package_json()
        print(f"[TempMonitor] Using ESPTool: {esptool_py}")
        env.Replace(UPLOADER=esptool_py)


try:
    setup_esptool()
except Exception as e:
    print(f"Error: {e}")
