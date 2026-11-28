"""Compare ordered pre-DMA block fingerprints from two six-voice, two-kit replays."""
import struct
import subprocess
import sys
from pathlib import Path

def read(directory):
    p = Path(directory)
    assert "skipped 0 cycles" in (p / "run.log").read_text()
    symbols = {}
    for line in subprocess.check_output(["readelf", "-Ws", p / "bench.elf"], text=True).splitlines():
        f = line.split()
        if len(f) >= 8 and f[3] == "OBJECT":
            symbols[f[7]] = int(f[1], 16) - 0x1C00000
    ram = (p / "ram.bin").read_bytes()
    def read_value(name, count):
        return struct.unpack_from(f"<{count}I", ram, symbols[name])
    assert read_value("bench_loaded_hits", 2) == (132, 132)
    assert read_value("bench_s1_only", 9) == (0, 65536, 12, 98304, 163840, 6, 0, 0, 0)
    assert read_value("bench_s1_digest_count", 1) == (160,)
    return read_value("bench_s1_digest", 160)

a, b = map(read, sys.argv[1:])
changes = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
print(f"Different pre-DMA block fingerprints: {len(changes)}/160; first indices: {changes[:12]}")
raise SystemExit(bool(changes))
