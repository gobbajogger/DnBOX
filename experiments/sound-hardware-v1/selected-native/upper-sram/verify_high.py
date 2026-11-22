#!/usr/bin/env python3
"""Verify the linked upper-SRAM layout and all four 42ae56e1 scene receipts."""
from argparse import ArgumentParser
from hashlib import sha256
from pathlib import Path
import gzip
import json
import re
import subprocess

def read(path):
    return gzip.open(path, "rb").read() if path.suffix == ".gz" else path.read_bytes()

def linked_sections(elf):
    rows = subprocess.check_output(["readelf", "-SW", elf], text=True)
    result = {}
    for name, address, size in re.findall(
        r"\[\s*\d+\]\s+(\.\S+)\s+\S+\s+([\da-f]+)\s+[\da-f]+\s+([\da-f]+)", rows
    ):
        result[name] = (int(address, 16), int(size, 16))
    return result

def checked_build(build, shots, model):
    elf = build / model / "x0x.elf"
    package = build / model / "x0x.fwsc"
    sections = linked_sections(elf)
    ble, ble_size = sections[".ble_ram"]
    hi, hi_size = sections[".audiohigh"]
    xp, xp_size = sections[".xpool"]
    assert ble == 0x01c7c400 and ble + ble_size <= hi
    assert hi == 0x01c7d400 and hi_size == 0x2880
    assert hi + hi_size <= 0x01c7fd00 < 0x01c7fd4c
    assert xp == 0x01c75200 and xp + xp_size <= 0x01c77600
    symbols = subprocess.check_output(["nm", "-S", elf], text=True, stderr=subprocess.DEVNULL)
    for symbol, address, size in (("original_bass", hi, hi_size), ("d808", None, 0xe7c)):
        match = re.search(rf"^([\da-f]+) ([\da-f]+) \w {symbol}$", symbols, re.M)
        assert match and int(match[2], 16) == size
        linked = int(match[1], 16)
        assert (linked == address if address is not None else xp <= linked <= xp + xp_size - size)
    result = {"elf": sha256(read(elf)).hexdigest(), "fwsc": sha256(read(package)).hexdigest(),
              "pool_bytes": sections[".pool"][1], "xpool_bytes": xp_size,
              "audiohigh_bytes": hi_size, "ble_bytes": ble_size, "scenes": {}}
    assert 344064 - sections[".pool"][1] - 8192 == 16140
    for scene in (("01", "32") if model == "elite" else ("10", "23")):
        d = shots / model / ("heavy" + scene)
        full = json.loads((d / "results.json").read_text())
        shot = full["shot"]
        assert full["elf_sha256"] == result["elf"] and full["fwsc_sha256"] == result["fwsc"]
        assert full["passed"] and shot["strict"] == 1 and shot["duration_ms"] == 8000 and shot["events"] == 922
        assert shot["max_us"] < 11609 and shot["raw_late"] == shot["audio_xruns"] == 0
        ram = read(next((d / "heavy").glob("ram.bin*")))
        base = 0x01c00000
        active = sum(v != 0 for v in ram[hi-base:hi+hi_size-base])
        assert active and not any(ram[ble+ble_size-base:hi-base])
        assert not any(ram[hi+hi_size-base:0x7fd4c])
        result["scenes"][scene] = {"max_us":shot["max_us"],"raw_late":shot["raw_late"],
                                   "xruns":shot["audio_xruns"],"nonzero_engine_bytes":active,
                                   "pcm_sha256":shot["pcm_sha256"]}
    return result

if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("builds", type=Path)
    parser.add_argument("shots", type=Path)
    args = parser.parse_args()
    print(json.dumps({m:checked_build(args.builds, args.shots, m) for m in ("elite", "pro")}, indent=2))
