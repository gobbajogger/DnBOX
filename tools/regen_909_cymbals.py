#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Rebuild the encoded 909 cymbals and source checksums from the original WAVs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

from gen_drum_samples import ASSETS, SAMPLES, SRC, read_wav


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="destination, e.g. assets/909/encoded")
    dest = parser.parse_args().output
    dest.mkdir(parents=True, exist_ok=True)
    manifest = {}
    with tempfile.TemporaryDirectory(dir=dest) as temporary:
        work = Path(temporary).resolve()
        exe = work / "encode"
        subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O3", "-I", str(SRC/"firmware/src/dsp"),
                        str(SRC/"tools/encode_909_cymbals.cpp"), "-o", str(exe)], check=True)
        for _, filename in SAMPLES:
            wav = ASSETS / filename
            pcm, _ = read_wav(wav)
            data = subprocess.run([str(exe)], input=struct.pack(f"<{len(pcm)}h", *pcm),
                                  stdout=subprocess.PIPE, check=True).stdout
            if len(data) != (len(pcm) + 1) // 2:
                raise ValueError(f"{filename}: wrong encoded length")
            (work / (wav.stem+".adpcm")).write_bytes(data)
            manifest[wav.stem] = {"wav_sha256": hashlib.sha256(wav.read_bytes()).hexdigest(),
                                  "adpcm_sha256": hashlib.sha256(data).hexdigest()}
        (work/"manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n")
        for stem in manifest:
            name = stem+".adpcm"
            (work/name).replace(dest/name)
        (work/"manifest.json").replace(dest/"manifest.json")


if __name__ == "__main__":
    main()
