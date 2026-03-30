import os
import urllib.request
import zipfile
import shutil
import json
from os.path import join, exists, basename

Import("env")

ESPTOOL_VERSION = "5.1.0"
URL = f"https://github.com/espressif/esptool/archive/refs/tags/v{ESPTOOL_VERSION}.zip"

def _get_pio_core_dir():
    try:
        piohome = env.subst("$PIOHOME_DIR")
        if piohome and "$" not in piohome:
            return piohome
    except Exception:
        pass
    for var in ("PLATFORMIO_CORE_DIR", "PIOHOME_DIR", "PLATFORMIO_HOME_DIR"):
        value = os.environ.get(var)
        if value:
            return value
    return join(os.path.expanduser("~"), ".platformio")

CORE_DIR = _get_pio_core_dir()
PACKAGES_DIR = join(CORE_DIR, "packages")
DOWNLOADS_DIR = join(CORE_DIR, "downloads")
ESPTOOL_DIR = join(PACKAGES_DIR, f"tool-esptoolpy-{ESPTOOL_VERSION}")
ZIP_PATH = join(DOWNLOADS_DIR, basename(URL))
PROJECT_DIR = env.subst("$PROJECT_DIR")
LEGACY_ESPTOOL_DIR = join(PROJECT_DIR, "tools", "esptoolspy")
LEGACY_ESPTOOL_PY = join(LEGACY_ESPTOOL_DIR, "esptool.py")

PACKAGE_JSON = {
    "name": "tool-esptoolpy",
    "version": ESPTOOL_VERSION,
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
        if not exists(ESPTOOL_DIR):
            os.makedirs(ESPTOOL_DIR)
        with open(package_path, "w") as f:
            json.dump(PACKAGE_JSON, f, indent=2)
        print(f"[TempMonitor] Generated esptool package.json")

def setup_esptool():
    if not exists(PACKAGES_DIR):
        os.makedirs(PACKAGES_DIR)
    if not exists(DOWNLOADS_DIR):
        os.makedirs(DOWNLOADS_DIR)
    esptool_py = join(ESPTOOL_DIR, "esptool.py")
    if not exists(esptool_py):
        migrated = False
        if exists(LEGACY_ESPTOOL_PY) and not exists(ESPTOOL_DIR):
            print("[TempMonitor] Migrating esptool from project tools/esptoolspy...")
            try:
                shutil.copytree(LEGACY_ESPTOOL_DIR, ESPTOOL_DIR)
                esptool_py = join(ESPTOOL_DIR, "esptool.py")
                migrated = exists(esptool_py)
            except Exception as e:
                print(f"[TempMonitor] Error migrating ESPTool: {e}")
        if not migrated and exists(ESPTOOL_DIR):
            shutil.rmtree(ESPTOOL_DIR)
        if not migrated and not exists(esptool_py):
            if not exists(ZIP_PATH):
                print("[TempMonitor] Downloading ESPTool...")
                try:
                    urllib.request.urlretrieve(URL, ZIP_PATH)
                except Exception as e:
                    print(f"[TempMonitor] Error downloading ESPTool: {e}")
                    return
            print("[TempMonitor] Extracting ESPTool...")
            try:
                with zipfile.ZipFile(ZIP_PATH, "r") as zip_ref:
                    extract_temp = join(DOWNLOADS_DIR, f"temp_esptool_{ESPTOOL_VERSION}")
                    if exists(extract_temp):
                        shutil.rmtree(extract_temp)
                    zip_ref.extractall(extract_temp)
                    extracted_name = os.listdir(extract_temp)[0]
                    shutil.move(join(extract_temp, extracted_name), ESPTOOL_DIR)
                    shutil.rmtree(extract_temp)
            except Exception as e:
                print(f"[TempMonitor] Error extracting ESPTool: {e}")
                return
    if exists(esptool_py):
        generate_package_json()
        print(f"[TempMonitor] Using ESPTool: {esptool_py}")
        env.Replace(UPLOADER=esptool_py)


try:
    setup_esptool()
except Exception as e:
    print(f"Error: {e}")
