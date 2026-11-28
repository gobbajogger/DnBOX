"""Read the witness-only counters from a strict native build (not timing evidence)."""
from pathlib import Path
import struct
import subprocess
import sys
p = Path(sys.argv[1])
tab = {}
for t in subprocess.check_output(["readelf", "-Ws", p / "bench.elf"], text=True).splitlines():
    f = t.split()
    if len(f) >= 8 and f[3] == "OBJECT":
        tab[f[7]] = int(f[1], 16) - 0x01c00000
ram = (p / "ram.bin").read_bytes()
ready = struct.unpack_from("<4I", ram, tab["bench_env_share"])
print("held_shared, held_fallback, other_shared, other_fallback =", ready)
assert sum(ready[:2]) == 65536
assert ready[0] > 0
