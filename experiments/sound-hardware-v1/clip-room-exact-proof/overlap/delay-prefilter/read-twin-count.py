"""Read the maximum completed half with the diagnostic prefilter counters."""
import json, struct, subprocess, sys
from pathlib import Path

elf, ram = sys.argv[1:3]
r = Path(ram).read_bytes()
s = {l.split()[-1]: int(l.split()[0], 16) - 0x1c00000
     for l in subprocess.check_output(["nm", "-S", elf], text=True).splitlines()
     if len(l.split()) >= 4}
h = struct.unpack_from("<I", r, s["lazy_worst_half"])[0]
v = struct.unpack_from("<8I", r, s["twin_worst"])
print(json.dumps(dict(half=h, spans=[dict(eligible_frames=x, calls_saved=y)
                                     for x,y in zip(v[::2],v[1::2]) if x])))
