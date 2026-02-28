import os
import urllib.request
import zipfile
from os.path import join, exists
import sys

Import("env")

PROJECT_DIR = env.subst("$PROJECT_DIR")
TOOLS_DIR = join(PROJECT_DIR, "tools")
GCC_BASE_DIR = join(TOOLS_DIR, "gcc15") 
TOOLCHAIN_ROOT = join(GCC_BASE_DIR, "xtensa-esp-elf") 
BIN_DIR = join(TOOLCHAIN_ROOT, "bin")

URL = "https://github.com/espressif/crosstool-NG/releases/download/esp-15.2.0_20251204/xtensa-esp-elf-15.2.0_20251204-x86_64-w64-mingw32.zip"
ZIP_PATH = join(TOOLS_DIR, "gcc15.zip")

def setup_toolchain():
    if not exists(GCC_BASE_DIR): os.makedirs(GCC_BASE_DIR)
    if not exists(BIN_DIR):
        if not exists(ZIP_PATH):
            print(f"[TempMonitor] Downloading Toolchain...")
            try:
                urllib.request.urlretrieve(URL, ZIP_PATH)
            except Exception as e:
                sys.stderr.write(f"Error downloading: {e}\n")
                return
        print("[TempMonitor] Extracting Toolchain...")
        try:
            with zipfile.ZipFile(ZIP_PATH, 'r') as zip_ref:
                zip_ref.extractall(GCC_BASE_DIR)
        except Exception as e:
            sys.stderr.write(f"Error extracting: {e}\n")
            return

    if exists(BIN_DIR):
        print(f"[TempMonitor] Using Toolchain at: {BIN_DIR}")
        env.PrependENVPath("PATH", BIN_DIR)
        p = "xtensa-esp32-elf-"
        exe = ".exe" if os.name == 'nt' else ""
        env.Replace(
            AR      = join(BIN_DIR, f"{p}gcc-ar{exe}"),
            AS      = join(BIN_DIR, f"{p}as{exe}"),
            CC      = join(BIN_DIR, f"{p}gcc{exe}"),
            CXX     = join(BIN_DIR, f"{p}g++{exe}"),
            GDB     = join(BIN_DIR, f"{p}gdb{exe}"),
            LD      = join(BIN_DIR, f"{p}ld{exe}"),
            RANLIB  = join(BIN_DIR, f"{p}gcc-ranlib{exe}"),
            SIZETOOL= join(BIN_DIR, f"{p}size{exe}"),
            OBJDUMP = join(BIN_DIR, f"{p}objdump{exe}"),
        )
        print(f"[TempMonitor] Forced Compiler: {join(BIN_DIR, f'{p}g++{exe}')}")
    else:
        sys.stderr.write(f"[TempMonitor] Error: Bin directory not found at {BIN_DIR}\n")

try:
    setup_toolchain()
except Exception as e:
    sys.stderr.write(f"[TempMonitor] Script Error: {e}\n")