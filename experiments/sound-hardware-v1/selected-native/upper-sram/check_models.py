#!/usr/bin/env python3
"""Compare SMK build products and legacy object files, and inspect the FM-1 layout."""
from argparse import ArgumentParser
from hashlib import sha256
from pathlib import Path
import json
import re
import subprocess

def digest(path):
    return sha256(path.read_bytes()).hexdigest()

def sections(path):
    data = subprocess.check_output(["readelf", "-SW", path], text=True)
    return {s: [int(base, 16), int(size, 16)] for s, base, size in re.findall(
        r"\[\s*\d+\]\s+(\.\S+)\s+\S+\s+([\da-f]+)\s+[\da-f]+\s+([\da-f]+)", data)}

parser = ArgumentParser(description=__doc__)
for key in ("passing", "scoped", "native", "baseline_objects", "baseline_mke_log", "scoped_mke_log"):
    parser.add_argument(key, type=Path)
args = parser.parse_args()
answer = {}
for model in ("elite", "pro"):
    products = {}
    for name in ("x0x.elf", "x0x.fwsc"):
        a, b = args.passing / model / name, args.scoped / model / name
        assert a.read_bytes() == b.read_bytes()
        products[name] = digest(b)
    answer[model] = products
ref = json.loads(args.baseline_objects.read_text())
mke_dir = args.native / "smk37-mke"
objects = {str(p.relative_to(mke_dir)): digest(p) for p in mke_dir.rglob("*.o") if 'loader' not in p.parts}
assert objects == ref and len(objects) == 76
for path in (args.baseline_mke_log, args.scoped_mke_log):
    assert "region `XIP' overflowed by 10060 bytes" in path.read_text()
answer['mke'] = {'matching_objects': len(ref), 'unchanged_xip_overflow_bytes': 10060}
fm1 = args.native / 'x0x.elf'
layout = sections(fm1)
assert layout['.xpool'] == [0x01c75200,5120] and layout['.audiohigh'][1] == 0
assert layout['.pool'] == [0x01c20000,331596]
answer['fm1'] = {'elf': digest(fm1), 'package': digest(args.native / 'x0x.fwsc'),
                 'pool_bytes': layout['.pool'][1], 'xpool_bytes': layout['.xpool'][1],
                 'audiohigh_bytes': layout['.audiohigh'][1]}
print(json.dumps(answer, indent=2))
