#!/usr/bin/env python3
"""Recover only original mono columns from the exact preserved PCM archive.

No engine source or candidate algorithm is read. Every archive, compressed
member, paired recording and extracted original column is hash-verified.
"""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import os
import sys
import tarfile

import numpy as np

HERE = Path(__file__).resolve().parent
ARCHIVE_BYTES = 211988480
ARCHIVE_SHA = "d5d11628a390f19d99783b1fb805979d371f5971efa01737c00bbc1e359b5403"
PLAN_SHA = "e8049a0247019d211ec9f96636073e6839ee321135fdf16001065dd54b565a9d"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="New original-mono directory")
    parser.add_argument("--case", help="One exact physical case; otherwise recover all 60")
    args = parser.parse_args()
    plan_data = (HERE / "reference_packet/case_plan.json").read_bytes()
    require(digest(plan_data) == PLAN_SHA, "Case plan changed")
    require(args.archive.stat().st_size == ARCHIVE_BYTES, "Archive size differs")
    with args.archive.open("rb") as stream:
        actual = hashlib.file_digest(stream, "sha256").hexdigest()
    require(actual == ARCHIVE_SHA, "Archive hash differs")
    cases = [c for c in json.loads(plan_data)["cases"]
             if c["case_id"].startswith("physical-gain/")]
    require(len(cases) == len({c["case_id"] for c in cases}) == 60, "Case coverage differs")
    if args.case:
        cases = [c for c in cases if c["case_id"] == args.case]
        require(len(cases) == 1, "Unknown physical case")
    output = args.output.absolute()
    require(not os.path.lexists(output), "Output already exists, including symlinks")
    require(all(not p.is_symlink() for p in output.parents), "Symlink output parent")
    output.mkdir(parents=True, exist_ok=False)
    recovered = []
    with tarfile.open(args.archive, "r:") as archive:
        members = archive.getmembers()
        for case in cases:
            name = case["case_id"].split("/", 1)[1]
            member_name = "sound-pcm-preservation/physical-gain/" + name + ".f32.gz"
            matches = [m for m in members if m.name == member_name]
            require(len(matches) == 1 and matches[0].isfile(), "Invalid archive member")
            member = matches[0]
            require(member.size == case["historical_gzip_bytes"], "Compressed member size differs")
            stream = archive.extractfile(member)
            require(stream is not None, "Missing archive stream")
            compressed = stream.read()
            require(digest(compressed) == case["historical_gzip_sha256"], "Compressed member differs")
            pair = gzip.decompress(compressed)
            require(len(pair) == 2116608 and digest(pair) == case["historical_paired_pcm_sha256"],
                    "Historical paired PCM differs")
            original = np.frombuffer(pair, dtype="<f4").reshape(-1, 2)[:, 0]
            mono = original.tobytes()
            require(len(mono) == 1058304 and digest(mono) == case["current_reference_mono_sha256"],
                    "Extracted original column differs")
            require(np.isfinite(original).all(), "Original PCM is nonfinite")
            target = output / (name + ".original-mono.f32le")
            with target.open("xb") as stream:
                stream.write(mono)
            recovered.append({"case_id": case["case_id"], "name": target.name,
                              "bytes": len(mono), "sha256": digest(mono)})
    receipt = {"archive_sha256": actual, "plan_sha256": PLAN_SHA,
               "files": recovered, "file_count": len(recovered),
               "original_render": "NOT_RUN", "candidate_column_exported": False}
    with (output / "RECOVERY.json").open("x") as stream:
        json.dump(receipt, stream, indent=2)
        stream.write("\n")
    print(json.dumps({"status": "RECOVERED_VERIFIED", "files": len(recovered),
                      "output": str(output), "archive_sha256": actual}, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, OSError, ValueError, KeyError, tarfile.TarError) as error:
        print("RECOVERY ERROR: " + str(error), file=sys.stderr)
        raise SystemExit(2)
