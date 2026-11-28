"""Decode trace-only per-subspan 808 admissions for the worst render."""
import json
import struct
import subprocess
import sys
from pathlib import Path

elf, ram = sys.argv[1:3]
syms = {l.split()[-1]: int(l.split()[0], 16) - 0x1c00000
        for l in subprocess.check_output(["nm", "-S", elf], text=True).splitlines()
        if len(l.split()) >= 4}
b = Path(ram).read_bytes()
slots = [struct.unpack_from("<3I", b, syms["schmitt_worst"] + 12*i) for i in range(4)]
print(json.dumps([dict(attempts=x, admitted=y, frames=n) for x, y, n in slots if n]))
