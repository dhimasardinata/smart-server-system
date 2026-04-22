"""
Jalankan semua contoh tes satu per satu.
"""
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
SCRIPTS = sorted(SCRIPT_DIR.glob("test_*.py"))

passed = 0
failed = 0

# Setiap file test dijalankan sendiri supaya mudah melihat hasilnya.
for script in SCRIPTS:
    if script.name == "run_all.py":
        continue
    print(f"\n{'='*60}")
    print(f"  {script.name}")
    print(f"{'='*60}")
    result = subprocess.run(
        [sys.executable, str(script)],
        cwd=str(SCRIPT_DIR),
        capture_output=True,
        text=True,
    )
    print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="")
    if result.returncode == 0 and '"ok":true' in result.stdout:
        passed += 1
    else:
        failed += 1
        print("  *** GAGAL ***")

print(f"\n{'='*60}")
print(f"  HASIL: {passed} berhasil, {failed} gagal dari {passed + failed} skenario")
print(f"{'='*60}")
