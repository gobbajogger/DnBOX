#!/usr/bin/env python3
"""Compare complete stereo captures produced for s4-inputs.json at unity gain."""
import argparse
import hashlib
import json
from pathlib import Path

import numpy as np

SPEC = Path(__file__).with_name("s4-inputs.json")


def channel_score(reference, candidate, rate, band):
    """Use the factory comparison's 2048-sample Hann windows, hop and floor."""
    a, b = reference.astype(np.float64), candidate.astype(np.float64)
    if not np.any(a) or not np.any(b):
        same = np.array_equal(a, b)
        return (0.0, 0.0) if same else (None, None)
    window = np.hanning(2048)

    def spectrum(x):
        windows = np.lib.stride_tricks.sliding_window_view(x, len(window))[::512]
        return np.sqrt(np.mean(abs(np.fft.rfft(windows * window)) ** 2, axis=0))

    sa, sb = spectrum(a), spectrum(b)
    freq = np.fft.rfftfreq(len(window), 1.0 / rate)
    floor = max(1e-12, np.max(sa) * 0.0001)
    use = (freq >= band[0]) & (freq <= band[1]) & ((sa > floor) | (sb > floor))
    level = 10 * np.log10(np.mean(b * b) / np.mean(a * a))
    if not np.any(use):
        return float(level), None
    difference = 20 * np.log10(np.maximum(sb[use], floor) / np.maximum(sa[use], floor))
    return float(level), float(np.sqrt(np.mean(difference * difference)))


def capture(directory, row, spec, spec_hash, arm):
    receipt = json.loads((directory / (row["program_uid"] + ".json")).read_text())
    for key, expected in {
        "arm": arm, "bank_uuid": row["bank_uuid"], "program_uid": row["program_uid"],
        "record_sha256": row["record_sha256"], "s4_inputs_sha256": spec_hash,
        "sample_rate": spec["sample_rate"], "frames": spec["capture_frames"],
        "channels": spec["channels"], "format": "stereo-f32le",
        "complete_preset": True
    }.items():
        if receipt.get(key) != expected:
            raise ValueError(f"{arm} receipt disagrees on {key}")
    if arm == "original" and receipt.get("au_sha256") != "a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1":
        raise ValueError("wrong original AU executable")
    if arm == "dnbox" and not receipt.get("source_commit"):
        raise ValueError("missing candidate source commit")
    raw = (directory / (row["program_uid"] + ".f32le")).read_bytes()
    if (len(raw) != spec["capture_frames"] * spec["channels"] * 4 or
            hashlib.sha256(raw).hexdigest() != receipt.get("pcm_sha256")):
        raise ValueError(f"{arm} PCM length or digest differs")
    data = np.frombuffer(raw, "<f4").reshape(-1, 2)
    if not np.isfinite(data).all() or not np.any(data):
        raise ValueError(f"{arm} capture is nonfinite or entirely silent")
    return data, receipt["pcm_sha256"]


def compare(original, dnbox):
    raw_spec = SPEC.read_bytes()
    spec_hash = hashlib.sha256(raw_spec).hexdigest()
    spec = json.loads(raw_spec)
    results = []
    for row in spec["programs"]:
        result = {"bank": row["bank"], "program": row["program"], "uid": row["program_uid"]}
        arms = {}
        errors = []
        for arm, directory in [("original", original), ("dnbox", dnbox)]:
            try:
                arms[arm], result[arm + "_pcm_sha256"] = capture(directory, row, spec, spec_hash, arm)
            except FileNotFoundError as error:
                errors.append(f"{arm} missing {Path(error.filename).name}")
            except (ValueError, KeyError, TypeError) as error:
                errors.append(str(error))
        if errors:
            result.update(status="UNSCORED", errors=errors)
        else:
            scores = [channel_score(a, b, spec["sample_rate"], spec["band_hz"])
                      for a, b in zip(arms["original"].T, arms["dnbox"].T)]
            passed = all(level is not None and abs(level) <= spec["level_limit_db"] and
                         spectral is not None and spectral <= spec["spectral_limit_db"]
                         for level, spectral in scores)
            result.update(status="PASS" if passed else "FAIL",
                          channels=[dict(level_db=a, spectral_rms_db=b) for a, b in scores])
        results.append(result)
    return {"s4_inputs_sha256": spec_hash, "programs": results,
            "passed": sum(r["status"] == "PASS" for r in results),
            "scored": sum(r["status"] != "UNSCORED" for r in results)}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original", type=Path)
    parser.add_argument("dnbox", type=Path)
    args = parser.parse_args()
    result = compare(args.original, args.dnbox)
    print(json.dumps(result, indent=2, allow_nan=False))
    raise SystemExit(0 if result["passed"] == 14 else 2 if result["scored"] != 14 else 1)
