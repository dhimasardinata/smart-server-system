import os
import sys
import platform
import tarfile
import urllib.request
import zipfile
import shutil
from os.path import join, exists, basename

Import("env")

TOOLCHAIN_VERSION = "15.2.0_20251204"
BASE_URL = f"https://github.com/espressif/crosstool-NG/releases/download/esp-{TOOLCHAIN_VERSION}"
ASSET_URLS = {
    "linux-amd64": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-x86_64-linux-gnu.tar.xz",
    "linux-i686": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-i586-linux-gnu.tar.xz",
    "linux-arm64": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-aarch64-linux-gnu.tar.xz",
    "linux-armel": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-arm-linux-gnueabi.tar.xz",
    "linux-armhf": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-arm-linux-gnueabihf.tar.xz",
    "macos": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-x86_64-apple-darwin.tar.xz",
    "macos-arm64": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-aarch64-apple-darwin.tar.xz",
    "win32": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-i686-w64-mingw32.zip",
    "win64": f"{BASE_URL}/xtensa-esp-elf-{TOOLCHAIN_VERSION}-x86_64-w64-mingw32.zip",
}

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

def _detect_host():
    override = os.environ.get("ESP_TOOLCHAIN_HOST")
    if override:
        return override
    system = platform.system().lower()
    machine = platform.machine().lower()
    if system == "windows":
        if machine in ("amd64", "x86_64"):
            return "win64"
        if machine in ("x86", "i386", "i686"):
            return "win32"
        return "win64" if sys.maxsize > 2**32 else "win32"
    if system == "darwin":
        if machine in ("arm64", "aarch64"):
            return "macos-arm64"
        return "macos"
    if system == "linux":
        if machine in ("x86_64", "amd64"):
            return "linux-amd64"
        if machine in ("i386", "i686"):
            return "linux-i686"
        if machine in ("aarch64", "arm64"):
            return "linux-arm64"
        if machine.startswith(("armv7", "armv8")):
            return "linux-armhf"
        if machine.startswith(("armv6", "armv5", "arm")):
            return "linux-armel"
    return None

def _ensure_dirs(path):
    if not exists(path):
        os.makedirs(path)

def _download(url, dest):
    if not exists(dest):
        print("[TempMonitor] Downloading Toolchain...")
        try:
            urllib.request.urlretrieve(url, dest)
        except Exception as e:
            sys.stderr.write(f"Error downloading: {e}\n")
            return False
    return True

def _extract(archive_path, dest_dir):
    print("[TempMonitor] Extracting Toolchain...")
    try:
        if archive_path.endswith(".zip"):
            with zipfile.ZipFile(archive_path, "r") as zip_ref:
                zip_ref.extractall(dest_dir)
        else:
            with tarfile.open(archive_path, "r:*") as tar_ref:
                tar_ref.extractall(dest_dir)
        return True
    except Exception as e:
        sys.stderr.write(f"Error extracting: {e}\n")
        return False

CORE_DIR = _get_pio_core_dir()
PACKAGES_DIR = join(CORE_DIR, "packages")
DOWNLOADS_DIR = join(CORE_DIR, "downloads")
GCC_BASE_DIR = join(PACKAGES_DIR, f"toolchain-xtensa-esp-elf-{TOOLCHAIN_VERSION}")
TOOLCHAIN_ROOT = join(GCC_BASE_DIR, "xtensa-esp-elf")
BIN_DIR = join(TOOLCHAIN_ROOT, "bin")
PROJECT_DIR = env.subst("$PROJECT_DIR")
LEGACY_GCC_DIR = join(PROJECT_DIR, "tools", "gcc15")
LEGACY_TOOLCHAIN_ROOT = join(LEGACY_GCC_DIR, "xtensa-esp-elf")
LEGACY_BIN_DIR = join(LEGACY_TOOLCHAIN_ROOT, "bin")

def _migrate_legacy_toolchain():
    if exists(LEGACY_BIN_DIR) and not exists(TOOLCHAIN_ROOT):
        print("[TempMonitor] Migrating toolchain from project tools/gcc15...")
        try:
            shutil.copytree(LEGACY_TOOLCHAIN_ROOT, TOOLCHAIN_ROOT)
            return True
        except Exception as e:
            sys.stderr.write(f"[TempMonitor] Error migrating toolchain: {e}\n")
    return False

def setup_toolchain():
    host_key = _detect_host()
    url = ASSET_URLS.get(host_key)
    if not url:
        sys.stderr.write(f"[TempMonitor] Unsupported host for toolchain: {platform.system()} {platform.machine()}\n")
        return

    _ensure_dirs(PACKAGES_DIR)
    _ensure_dirs(DOWNLOADS_DIR)
    _ensure_dirs(GCC_BASE_DIR)

    if not exists(BIN_DIR):
        if _migrate_legacy_toolchain():
            pass
        if not exists(BIN_DIR):
            archive_path = join(DOWNLOADS_DIR, basename(url))
            if not _download(url, archive_path):
                return
            if not _extract(archive_path, GCC_BASE_DIR):
                return

    if exists(BIN_DIR):
        print(f"[TempMonitor] Using Toolchain at: {BIN_DIR}")
        env.PrependENVPath("PATH", BIN_DIR)
        p = "xtensa-esp32-elf-"
        exe = ".exe" if os.name == 'nt' else ""
        if not exists(join(BIN_DIR, f"{p}gcc{exe}")):
            alt = "xtensa-esp-elf-"
            if exists(join(BIN_DIR, f"{alt}gcc{exe}")):
                p = alt
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
