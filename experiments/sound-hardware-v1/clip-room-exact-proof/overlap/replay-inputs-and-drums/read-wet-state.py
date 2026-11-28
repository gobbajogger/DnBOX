"""Read forced-wet activation and controls from a matched Pro image."""
import json, struct, subprocess, sys
from pathlib import Path

elf, ram = sys.argv[1:3]
r = Path(ram).read_bytes()
s = {l.split()[-1]: int(l.split()[0], 16) - 0x1c00000
     for l in subprocess.check_output(["nm", "-S", elf], text=True).splitlines()
     if len(l.split()) >= 4}
live = struct.unpack_from("<I", r, s["s1c_render.wet_test_started"])[0]
vals = {k: struct.unpack_from("<f", r, s["original_bass"] + 5344 + (46+i) * 4)[0]
        for k, i in [("DLON", 37), ("DLMIX", 40), ("RVON", 41), ("DLT", 43), ("RVMIX", 44)]}
assert live == 1 and vals["DLON"] == vals["RVON"] == 1
assert vals["DLMIX"] == vals["RVMIX"] == .5 and abs(vals["DLT"] - .3) < 1e-6
print(json.dumps(dict(activated=live, part_b=vals)))
