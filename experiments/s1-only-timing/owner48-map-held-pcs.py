#!/usr/bin/env python3
"""Map weighted emulator PC observations by exact uninstrumented ELF ranges."""
import argparse
from collections import defaultdict
import subprocess

p = argparse.ArgumentParser(__doc__)
p.add_argument("elf")
p.add_argument("pcs")
a = p.parse_args()
sym = []
for line in subprocess.check_output(["readelf", "-sW", a.elf], text=True).splitlines():
    w = line.split()
    if len(w) >= 8 and w[3] == "FUNC" and w[6] != "UND":
        lo, size = int(w[1], 16), int(w[2])
        if size:
            sym.append((lo, lo + size, w[7]))
counts = defaultdict(lambda: defaultdict(int))
with open(a.pcs, encoding="ascii") as f:
    for line in f:
        core, addr, weight = line.split()
        if core not in ("0", "1"):   # m0/m1 are cache-line observations, not PC weights.
            continue
        addr, weight = int(addr, 16), int(weight)
        exact = sorted((hi-lo, name) for lo, hi, name in sym if lo <= addr < hi)
        name = exact[0][1] if exact else f"unmapped {addr & ~0xfff:#010x}"
        counts[core][name] += weight
for core in sorted(counts):
    total = sum(counts[core].values())
    print(f"core {core}: {total:,} weighted samples")
    for key, value in sorted(counts[core].items(), key=lambda kv: -kv[1])[:25]:
        print(f"{value:>12,} {100*value/total:>6.2f}% {key}")
