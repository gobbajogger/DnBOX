#!/usr/bin/env python3
"""Compare complete stereo captures produced for s4-inputs.json at unity gain."""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

from factory import canonical_bytes
from release_matrix import prepare_release

import numpy as np

SPEC = Path(__file__).with_name("s4-inputs.json")


AUDIT = SPEC.parent / "original/bundled-bank-audit.json"


def channel_score(reference, candidate, rate, band):
    """Use the factory comparison's 2048-sample Hann windows, hop and floor."""
    a, b = reference.astype(np.float64), candidate.astype(np.float64)
    if not np.any(a) or not np.any(b):
        return (0.0, 0.0) if np.array_equal(a, b) else (None, None)
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


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def json_object(raw):
    def unique_keys(pairs):
        obj = {}
        for key, value in pairs:
            if key in obj:
                raise ValueError(f"duplicate JSON key {key}")
            obj[key] = value
        return obj

    item = json.loads(raw, object_pairs_hook=unique_keys)
    if not isinstance(item, dict):
        raise ValueError("expected a JSON object")
    return item


def expect(actual, expected, label):
    # Keep booleans distinct from JSON numbers, including in event records.
    # Apple JSONSerialization emits integral floats as integers.
    def normalized(x):
        if isinstance(x, float) and x.is_integer():
            return int(x)
        if isinstance(x, list):
            return [normalized(v) for v in x]
        if isinstance(x, dict):
            return {k: normalized(v) for k, v in x.items()}
        return x

    if canonical_bytes(normalized(actual)) != canonical_bytes(normalized(expected)):
        raise ValueError(f"receipt disagrees on {label}")


def control_words(entries):
    if not isinstance(entries, list) or not entries:
        raise ValueError("empty parameter readback")
    words = {}
    for entry in entries:
        if not isinstance(entry, dict):
            raise ValueError("malformed parameter readback")
        name, value = entry.get("identifier"), entry.get("value")
        if not isinstance(name, str) or name in words or type(value) not in (float, int):
            raise ValueError("duplicate or malformed parameter readback")
        bits = struct.unpack("<I", struct.pack("<f", value))[0]
        if bits & 0x7f800000 == 0x7f800000:
            raise ValueError("nonfinite parameter readback")
        words[name] = bits
    return words


def check_state(arm, directory, row, checked, receipt, inputs):
    uid = row["program_uid"]
    if inputs is None:
        raise ValueError("authenticated factory input directory required")
    raw = inputs[uid] if isinstance(inputs, dict) else (inputs / (uid + ".json")).read_bytes()
    expect(digest(raw), checked["file_sha256"], "input file digest")
    selection = json_object(raw)
    source = selection["preset"]
    expect(selection["bank"]["uuid"], row["bank_uuid"], "input bank")
    expect(source["uid"], uid, "input UID")
    expect(digest(canonical_bytes(source)), row["record_sha256"], "input record digest")
    expect(receipt.get("input_file_sha256"), checked["file_sha256"], "input_file_sha256")
    active_raw = (directory / (uid + ".applied.json")).read_bytes()
    expect(digest(active_raw), receipt.get("applied_state_sha256"), "applied state digest")
    active = json_object(active_raw)
    if active.get("uid") != uid and (arm != "original" or active.get("originalUid") != uid):
        raise ValueError("applied UID mismatch")
    # Inspect non-parameter settings that influence playback, including tuning.
    for key in ("keyboardOctave", "modWheelDestination", "seqTempo", "microtonalMasterSet", "microtonalTuningName"):
        expect(active.get(key), source.get(key), "applied " + key)
    for key in ("authored_tempo", "applied_tempo"):
        expect(receipt.get(key), source["seqTempo"], key)
    authored = control_words(source["parameterValues"])
    expect(len(authored), checked["parameter_count"], "audited control count")
    snapshot = control_words(active["parameterValues"])
    expect({k: snapshot.get(k) for k in authored}, authored, "applied control words")
    details = receipt.get("applied_readback")
    if not isinstance(details, dict):
        raise ValueError("missing live readback")
    expect(details.get("qualified"), True, "qualified readback")
    for key in ("missing_active", "active_differences", "missing_live", "live_differences"):
        expect(details.get(key), [], key)
    legacy = details.get("unexposed_legacy")
    if not isinstance(legacy, list) or len(legacy) != len(set(legacy)):
        raise ValueError("malformed legacy controls")
    if legacy and (arm != "original" or not re.fullmatch(r"factoryPresets/[0-9]+", receipt["factory_route"])
                   or set(legacy) - ({"neonlimiterAmount", "neontubeRageAmount"} & set(authored))):
        raise ValueError("unqualified live control exceptions")
    live = {k: v for k, v in authored.items() if k not in legacy}
    expect(details.get("authored"), len(authored), "authored count")
    expect(details.get("live_matches"), len(live), "live_matches")
    expect(details.get("live_words"), live, "live control words")


def capture(directory, row, spec, spec_hash, arm, inputs=None, candidate_sha=None, candidate_commit=None,
            audit=None):
    uid = row["program_uid"]
    receipt = json_object((directory / (uid + ".json")).read_bytes())
    for key, expected in {
        "arm": arm, "bank_uuid": row["bank_uuid"], "program_uid": uid,
        "record_sha256": row["record_sha256"], "s4_inputs_sha256": spec_hash,
        "sample_rate": spec["sample_rate"], "frames": spec["capture_frames"],
        "channels": spec["channels"], "format": "stereo-f32le", "complete_preset": True,
        "block_frames": spec["block_frames"], "gain": spec["gain"], "reset": spec["reset"],
        "transport": spec["transport"], "first_event_frame": spec["events"][0]["frame"],
        "first_render_frame": 0, "event_count": len(spec["events"]), "scheduled_events": spec["events"],
    }.items():
        expect(receipt.get(key), expected, key)
    if audit is None:
        audit = json_object(AUDIT.read_bytes())
    checked = next((p for p in audit["programs"] if p["uid"] == uid), None)
    if not checked or checked["supplied_bank_uuid"] != row["bank_uuid"] or audit["spec_sha256"] != spec_hash:
        raise ValueError("release audit disagrees with matrix")
    if arm == "original":
        for key, expected in {"au_sha256": audit["au_sha256"], "release_au_sha256": audit["au_sha256"],
                              "bundled_bank_uuid": checked["bundled_bank_uuid"],
                              "component_version": audit["component_version"]}.items():
            expect(receipt.get(key), expected, key)
        route = receipt.get("factory_route")
        if not isinstance(route, str) or not re.fullmatch(r"fullState|factoryPresets/[0-9]+", route):
            raise ValueError("unknown original factory loader")
    else:
        if not candidate_commit or not re.fullmatch(r"[0-9a-f]{40}", candidate_commit) or not candidate_sha:
            raise ValueError("independent DnBOX source commit and production image are required")
        expect(receipt.get("source_commit"), candidate_commit, "source_commit")
        expect(receipt.get("firmware_sha256"), candidate_sha, "firmware_sha256")
    check_state(arm, directory, row, checked, receipt, inputs)
    raw = (directory / (uid + ".f32le")).read_bytes()
    if len(raw) != spec["capture_frames"] * spec["channels"] * 4 or digest(raw) != receipt.get("pcm_sha256"):
        raise ValueError(f"{arm} PCM length or digest differs")
    data = np.frombuffer(raw, "<f4").reshape(-1, spec["channels"])
    if not np.isfinite(data).all() or not np.any(data, axis=0).all():
        raise ValueError(f"{arm} PCM contains nonfinite values or a silent channel")
    return data, receipt["pcm_sha256"]


def compare(original, dnbox, inputs=None, candidate_image=None, candidate_commit=None, release_ipa=None):
    candidate_sha = None
    if candidate_image is not None:
        image = candidate_image.read_bytes()
        if not image:
            raise ValueError("empty DnBOX production image")
        candidate_sha = digest(image)
    if release_ipa is None:
        raw_spec = SPEC.read_bytes()
        audit = json_object(AUDIT.read_bytes())
    else:
        if inputs is not None:
            raise ValueError("choose an original release or exported matrix input")
        raw_spec, audit, inputs = prepare_release(release_ipa)
    spec_hash = hashlib.sha256(raw_spec).hexdigest()
    spec = json.loads(raw_spec)
    results = []
    for row in spec["programs"]:
        result = {"bank": row["bank"], "program": row["program"], "uid": row["program_uid"]}
        arms = {}
        errors = []
        for arm, directory in [("original", original), ("dnbox", dnbox)]:
            try:
                arms[arm], result[arm + "_pcm_sha256"] = capture(
                    directory, row, spec, spec_hash, arm, inputs, candidate_sha, candidate_commit, audit)
            except FileNotFoundError as error:
                errors.append(f"{arm} missing {Path(error.filename).name}")
            except (ValueError, KeyError, TypeError, OverflowError) as error:
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
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--inputs", type=Path, help="factory.py matrix output")
    source.add_argument("--release-ipa", "--release-app", type=Path, dest="release_ipa",
                        help="authenticate inputs from the original build-9 IPA or installed .app directory")
    parser.add_argument("--candidate-image", type=Path, help="independently identified DnBOX production executable")
    parser.add_argument("--candidate-commit", help="full source commit for that executable")
    args = parser.parse_args()
    result = compare(args.original, args.dnbox, args.inputs, args.candidate_image, args.candidate_commit, args.release_ipa)
    print(json.dumps(result, indent=2, allow_nan=False))
    raise SystemExit(0 if result["passed"] == 14 else 2 if result["scored"] != 14 else 1)
