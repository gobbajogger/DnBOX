"""Check actual sequencer hits of both kits during the held six-note interval."""
from pathlib import Path
import struct, subprocess, sys
p = Path(sys.argv[1])
ram = (p/"ram.bin").read_bytes()
symbol = {}
for line in subprocess.check_output(["readelf", "-Ws", p/"bench.elf"], text=True).splitlines():
    f = line.split()
    if len(f) >= 8 and f[3] == "OBJECT":
        symbol[f[7]] = int(f[1], 16) - 0x01c00000
def read(name, count):
    return struct.unpack_from(f"<{count}I", ram, symbol[name])
on, = read("bench_loaded_on", 1)
playing, = read("bench_loaded_run", 1)
k909, k808 = read("bench_loaded_hits", 2)
assert (on,playing)==(0,1) and min(k909,k808)>0, (on,playing,k909,k808)
assert "skipped 0 cycles" in (p/"run.log").read_text()
print(f"Sequencer running, actual drum hits while held: 909={k909}, 808={k808}")
