#!/usr/bin/env python3
"""Write only the test input words to ephemeral storage after authentication."""
from hashlib import sha256
from pathlib import Path
import struct
import sys

source, archive, output = map(Path, sys.argv[1:])
sys.path.insert(0, str(source / "experiments/sound-hardware-v1/sj6-factory"))
from factory import load_factory

if sha256(archive.read_bytes()).hexdigest() != "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8":
    raise SystemExit("Unexpected factory archive digest")
with output.open("w") as stream:
    stream.write("static const uint32_t source_division_words[] = {\n")
    for _, records in load_factory(archive).values():
        for record in records.values():
            value = next(f["value"] for f in record["parameterValues"] if f["identifier"] == "neonseqDivision")
            word = struct.unpack("<I", struct.pack("<f", value))[0]
            stream.write(f"    0x{word:08x}u,\n")
    stream.write("};\n")
