#!/usr/bin/env python3
"""Extract only an ephemeral mask oracle from the authenticated owner archive."""
from hashlib import sha256
import json
from pathlib import Path
import re
import sys
import zipfile

archive, output = map(Path, sys.argv[1:])
assert sha256(archive.read_bytes()).hexdigest() == "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
masks = []
with zipfile.ZipFile(archive) as z:
    for fname in sorted(z.namelist()):
        if not fname.startswith("J6presets/") or not fname.endswith(".superj6.bank.json"):
            continue
        for row in json.loads(z.read(fname))["presets"]:
            steps = {}
            for field in row["parameterValues"]:
                m = re.fullmatch(r"neonseqStep([0-9]+)Toggle", field["identifier"])
                if m:
                    i = int(m[1])
                    assert 1 <= i <= 16
                    assert i not in steps and type(field["value"]) in (float, int) and field["value"] in (0, 1)
                    steps[i] = int(field["value"])
            assert not steps or len(steps) == 16
            masks.append(sum(bit << (i - 1) for i, bit in steps.items()) if steps else 0x10000)
assert len(masks) == 640 and masks.count(0x10000) == 147
assert sum(16 - v.bit_count() for v in masks if v < 0x10000) == 377
output.write_text("static const unsigned source_toggle_mask[] = {\n" +
                  "".join(f"0x{v:05x}u,\n" for v in masks) + "};\n")
