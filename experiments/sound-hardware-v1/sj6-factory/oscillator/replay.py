#!/usr/bin/env python3
"""Compare the compact core's pre-filter signal to the archived analytic reference."""
import argparse
import ctypes as ct
import hashlib
import json
from pathlib import Path
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
from compare import channel_score
from factory import load_factory, select, canonical_bytes

REF = "4ea86b9e869011cae761709d13569b7bc32980a8"
HEADER = "experiments/superj6-original-reference-20261115/sampler/DunneCore/Common/OriginalAnalyticOscillator.h"
REF_HASH = "546c8b94493fd8d22ccc575e7da932b8cc5dca5207385ebdca2e02d4000f8ee3"
F32 = ct.POINTER(ct.c_float)
NOTES = {
    "controls": (48, 60, 64, 67),
    "range": (24, 36, 43, 55, 59, 71, 72, 76, 79, 84, 96),
    "holdout": (37, 50, 62, 68, 75, 82, 91, 101),
}
AMOUNTS = {
    "controls": (0., .12632030248641968, .5043136477470398),
    "range": (0., .37, .83, 1.),
    "holdout": (.023, .71, .965),
}
FRAMES = 8192
SPEC = json.loads((HERE.parent / 's4-inputs.json').read_bytes())


def digest(data):
    return hashlib.sha256(data).hexdigest()


def build(compact, repo, out):
    header = subprocess.check_output(["git", "-C", repo, "show", f"{REF}:{HEADER}"])
    if digest(header) != REF_HASH:
        raise ValueError("reference input checksum differs")
    (out / "OriginalAnalyticOscillator.h").write_bytes(header)
    dsp = compact / "firmware/src/dsp"
    code = (dsp / "dnbx_core.c").read_text()
    observation = "x+=(lv->subphase"
    if code.count(observation) != 1:
        raise ValueError("could not locate oscillator output before sub oscillator")
    code = "void observe_oscillator(unsigned,unsigned,float);\n" + code.replace(
        observation, "observe_oscillator(i,l,x);\n                " + observation)
    (out / "core_tap.c").write_text(code)
    flags = ["-O2", "-fPIC", "-shared", "-ffp-contract=off", "-fno-fast-math"]
    subprocess.run(["c++", "-std=c++17", *flags, "-I", out, HERE / "reference.cpp",
                    "-o", out / "reference.so"], check=True)
    subprocess.run(["cc", "-std=c11", *flags, "-I", dsp, out / "core_tap.c",
                    dsp / "juno6.c", HERE / "candidate.c", "-o", out / "candidate.so"], check=True)
    reference = ct.CDLL(str(out / "reference.so")).reference_oscillator
    reference.argtypes = [ct.c_uint, ct.c_float, ct.c_float, F32, ct.c_size_t]
    library = ct.CDLL(str(out / "candidate.so"))
    candidate = library.candidate_oscillator
    candidate.argtypes = [ct.c_uint, ct.c_float, ct.c_float, ct.c_float, ct.c_uint,
                          F32, F32, ct.c_size_t, ct.c_uint]
    stock = library.stock_stereo
    stock.argtypes = [ct.c_uint, F32, ct.c_size_t]
    stock_hashes = []
    for mode in (0, 1):
        pcm = np.zeros(2048, np.float32)
        if not stock(mode, pcm.ctypes.data_as(F32), len(pcm) // 2) or not np.any(pcm):
            raise RuntimeError("legacy render failed")
        stock_hashes.append(digest(pcm.tobytes()))
    return reference, candidate, stock_hashes, {name: digest((dsp / name).read_bytes()) for name in
                                  ("dnbx_core.c", "dnbx_core.h", "fastmath.h", "juno6.c", "juno6.h")}


def cases(archive, suite):
    if archive is None:
        for layer in range(2):
            for stop in range(3):
                yield "control-" + str(stop), layer, np.float32(stop / 3), 0., 0.
            for amount in AMOUNTS[suite]:
                yield "control-pwm-" + str(amount), layer, 1., np.float32(amount), float(amount > 0)
        return
    if digest(archive.read_bytes()) != SPEC["archive_sha256"]:
        raise ValueError("expected the authenticated J6presets.zip")
    banks = load_factory(archive)
    catalog = json.loads((HERE.parent.parent / "mapping/results/implementer_brief/public_control_catalog.json").read_bytes())
    keys = {entry["id"]: entry["identifier"] for entry in catalog["controls"]}
    for row in SPEC["programs"]:
        record = select(banks, row["bank_uuid"], row["program_uid"])["preset"]
        if digest(canonical_bytes(record)) != row["record_sha256"]:
            raise ValueError(f"record checksum differs for {row['program_uid']}")
        fields = {p["identifier"]: np.float32(p["value"]) for p in record["parameterValues"]}
        for layer in range(2):
            # The original main preset pass defaults these two fields to +0.
            # This component excludes later LFO/live-control modulation.
            yield (row["program_uid"], layer, fields[keys[82 + layer]],
                   fields.get(keys[78 + layer], 0.), fields.get(keys[210 + layer], 0.))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compact", type=Path, required=True)
    parser.add_argument("--repo", type=Path, required=True)
    inputs = parser.add_mutually_exclusive_group()
    inputs.add_argument("--banks", type=Path)
    inputs.add_argument("--suite", choices=NOTES)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    reference, candidate, stock_hashes, inputs = build(args.compact, args.repo, out)
    suite = args.suite or "controls"
    rows = []
    for uid, layer, selector, amount, enable in cases(args.banks, suite):
        stop = int(3 * float(selector))
        width = .5 - .47 * float(amount) if stop == 3 and enable > .5 else .5
        for note in NOTES[suite]:
            a, b = np.zeros(FRAMES, np.float32), np.zeros(FRAMES, np.float32)
            increment = ct.c_float()
            if not candidate(layer, selector, amount, enable, note, ct.byref(increment),
                             b.ctypes.data_as(F32), FRAMES, 0):
                raise RuntimeError("compact render failed")
            with_peers = np.empty_like(b)
            if not candidate(layer, selector, amount, enable, note, ct.byref(increment),
                             with_peers.ctypes.data_as(F32), FRAMES, 1) or not np.array_equal(b, with_peers):
                raise RuntimeError("another oscillator changed the selected layer's signal")
            if not reference(stop, width, increment.value, a.ctypes.data_as(F32), FRAMES):
                raise RuntimeError("reference render failed")
            if not all(np.all(np.isfinite(x)) and np.any(x) for x in (a, b)):
                raise ValueError("non-finite or silent oscillator")
            rms, spectral = channel_score(a, b, SPEC["sample_rate"], SPEC["band_hz"])
            rows.append({"case": uid, "layer": layer + 1, "note": note, "shape": stop,
                         "selector": float(selector), "amount": float(amount), "enable": float(enable),
                         "increment": increment.value, "reference_width": width,
                         "reference_sha256": digest(a.tobytes()), "candidate_sha256": digest(b.tobytes()),
                         "level_db": rms, "spectral_db": spectral,
                         "dc_difference": float(np.mean(b.astype(float) - a)),
                         "correlation": float(np.corrcoef(a, b)[0, 1]),
                         "pass": abs(rms) <= SPEC["level_limit_db"] and spectral <= SPEC["spectral_limit_db"]})
    result = {"reference_commit": REF, "reference_header_sha256": REF_HASH, "candidate_files": inputs,
              "scope": "factory-main-pass base oscillator only" if args.banks else f"isolated oscillator {suite}",
              "stock_stereo_sha256_by_mode": stock_hashes,
              "frames_per_case": FRAMES, "rate": SPEC["sample_rate"],
              "unchanged_with_other_layer_and_voice": len(rows),
              "passes": sum(row["pass"] for row in rows), "total": len(rows), "cases": rows}
    (out / "result.json").write_text(json.dumps(result, indent=2, allow_nan=False) + "\n")
    print(f"{result['passes']}/{len(rows)} isolated oscillator comparisons within limits")
    return int(result["passes"] != len(rows))


if __name__ == "__main__":
    sys.exit(main())
