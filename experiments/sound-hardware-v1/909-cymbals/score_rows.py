#!/usr/bin/env python3
"""Apply the sound lane's unaligned level and spectral limits to full-path 909 mono renders."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

import numpy as np

sys.path.insert(0,str(Path(__file__).resolve().parent.parent/"sj6-factory"))
from compare import channel_score


def rows(path):
    with path.open("rb") as stream:
        while header := stream.read(8):
            if len(header)!=8:
                raise ValueError("incomplete row header")
            nlabel, frames = struct.unpack("<II", header)
            if nlabel > 64 or frames != 44100:
                raise ValueError("invalid row dimensions")
            label = stream.read(nlabel).decode()
            data = stream.read(8*frames)
            if len(data)!=8*frames:
                raise ValueError("short row")
            ref, port = np.frombuffer(data, dtype="<f4").reshape(2,frames)
            if not np.all(np.isfinite(ref)) or not np.all(np.isfinite(port)):
                raise ValueError("nonfinite PCM")
            level, spectrum = channel_score(ref, port, 44100, [20,16000])
            match = level is not None and spectrum is not None and abs(level)<=1 and spectrum<=3
            yield {"case":label, "level_db":level, "spectral_db":spectrum, "within_limits":match,
                   "silent_both":not np.any(ref) and not np.any(port),
                   "reference_sha256":hashlib.sha256(ref.tobytes()).hexdigest(),
                   "candidate_sha256":hashlib.sha256(port.tobytes()).hexdigest()}


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("pcm",type=Path)
    ap.add_argument("output",type=Path)
    args=ap.parse_args()
    result = list(rows(args.pcm))
    if not result or len({r["case"] for r in result})!=len(result):
        raise ValueError("empty or duplicate cases")
    args.output.write_text(json.dumps(result,indent=2,allow_nan=False)+"\n")
    for voice in ("CH","OH","CR","RD"):
        group=[r for r in result if r["case"].startswith(voice+" ")]
        print(voice,len(group),sum(r["within_limits"] for r in group),sum(r["silent_both"] for r in group),
              round(max(abs(r["level_db"]) for r in group),4),
              round(max(r["spectral_db"] for r in group),4))
        print([(r["case"],round(r["level_db"],3),round(r["spectral_db"],3)) for r in group[:2]])

if __name__ == "__main__":
    main()
