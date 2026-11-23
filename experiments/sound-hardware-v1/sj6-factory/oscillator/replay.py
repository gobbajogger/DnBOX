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
from compare import AUDIT, channel_score, json_object
from factory import load_factory, select, canonical_bytes
from release_matrix import prepare_release
import remote_fields

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
# Frozen from the pre-correction compact tree, d371377ada1f15a75ce33868c7d04172b273f8bb.
# These two probes check compact legacy quality modes independently of factory scores.
LEGACY_HASHES = (
    "0bf8c021876aaeda60c4288dbaa8cb6eab99046e7459512675b23e7356b30fab",
    "10f4073e2f0aedaddb1db3621689e647131e58f004a043e0579eb05140ad79ba",
)
FRAMES = 8192
SPEC_FILE = HERE.parent / "s4-inputs.json"
SPEC = json.loads(SPEC_FILE.read_bytes())


def digest(data):
    return hashlib.sha256(data).hexdigest()


def build(compact, repo, out, after_stock_pad=False):
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
    if after_stock_pad:
        flags.append("-DSJ6_PROBE_STOCK_PAD")
    subprocess.run(["cc", "-std=c11", *flags, "-DX0X_SUPERJ6_FACTORY=1", "-DDNBX_DIAGNOSTIC_COUNTERS=0",
                    "-I", dsp, out / "core_tap.c",
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
        actual = digest(pcm.astype("<f4", copy=False).tobytes())
        if actual != LEGACY_HASHES[mode]:
            raise ValueError(f"compact legacy mode {mode} stereo hash differs: {actual}")
        stock_hashes.append(actual)
    inputs = ["dnbx_core.c", "dnbx_core.h", "fastmath.h", "juno6.c", "juno6.h"]
    if (dsp / "juno6_factory.inc").exists():
        inputs.append("juno6_factory.inc")
    return reference, candidate, stock_hashes, {name: digest((dsp / name).read_bytes()) for name in inputs}


def selected_record(directory, row, audited):
    uid = row["program_uid"]
    if audited["uid"] != uid or audited["supplied_bank_uuid"] != row["bank_uuid"]:
        raise ValueError(f"release audit disagrees with S4 identity for {uid}")
    path = directory / f"{uid}.json"
    try:
        raw = path.read_bytes()
    except FileNotFoundError as error:
        raise ValueError(f"missing selected S4 record {path}") from error
    item = json_object(raw)
    if "uid" not in item and "preset" in item:
        bank = item.get("bank")
        if not isinstance(bank, dict) or bank.get("uuid") != row["bank_uuid"]:
            raise ValueError(f"selected bank UUID differs for {uid}")
        if digest(raw) != audited["file_sha256"]:
            raise ValueError(f"selected matrix file checksum differs for {uid}")
        return item["preset"]
    return item


def cases(archive, suite, selected=None, release=None):
    if archive is None and selected is None and release is None:
        for layer in range(2):
            for stop in range(3):
                yield "control-" + str(stop), layer, np.float32(stop / 3), 0., 0., {}
            for amount in AMOUNTS[suite]:
                yield "control-pwm-" + str(amount), layer, 1., np.float32(amount), float(amount > 0), {}
        return
    if release is not None:
        release_spec, _, released_files = release
        spec = json.loads(release_spec)
    else:
        spec = SPEC
    if selected is not None:
        audit = json_object(AUDIT.read_bytes())
        if audit["spec_sha256"] != digest(SPEC_FILE.read_bytes()):
            raise ValueError("release audit and S4 manifest disagree")
        audited = {entry["uid"]: entry for entry in audit["programs"]}
    elif archive is not None:
        if digest(archive.read_bytes()) != SPEC["archive_sha256"]:
            raise ValueError("expected the authenticated J6presets.zip")
        banks = load_factory(archive)
    catalog = json.loads((HERE.parent.parent / "mapping/results/implementer_brief/public_control_catalog.json").read_bytes())
    keys = {entry["id"]: entry["identifier"] for entry in catalog["controls"]}
    for row in spec["programs"]:
        uid = row["program_uid"]
        if release is not None:
            record = json_object(released_files[uid])["preset"]
        elif selected is not None:
            record = selected_record(selected, row, audited[uid])
        else:
            record = select(banks, row["bank_uuid"], uid)["preset"]
        if not isinstance(record, dict) or record.get("uid") != uid:
            raise ValueError(f"expected complete preset object with UID {uid}")
        if digest(canonical_bytes(record)) != row["record_sha256"]:
            raise ValueError(f"record checksum differs for {row['program_uid']}")
        yield from authored_layers(record, keys)


def authored_layers(record, keys):
    fields = {}
    for parameter in record["parameterValues"]:
        # Original main-pass lookup takes the first matching identifier.
        fields.setdefault(parameter["identifier"], np.float32(parameter["value"]))
    for layer in range(2):
        amount = fields.get(keys[78 + layer])
        enable = fields.get(keys[210 + layer])
        # Absent controls default independently to positive zero in the main
        # pass. A live route snapshot and receiver are needed for later stages.
        details = {"authored_pwm_amount": None if amount is None else float(amount),
                   "authored_pwm_enable": None if enable is None else float(enable)}
        yield (record["uid"], layer, fields[keys[82 + layer]],
               0. if amount is None else amount, 0. if enable is None else enable, details)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compact", type=Path, required=True)
    parser.add_argument("--repo", type=Path, required=True)
    inputs = parser.add_mutually_exclusive_group()
    inputs.add_argument("--banks", type=Path)
    inputs.add_argument("--release-ipa", type=Path, help="original build-9 app with its bundled factory banks")
    inputs.add_argument("--selected-records", type=Path, metavar="DIR",
                        help="14 <UID>.json files from factory.py matrix or their complete extracted presets")
    inputs.add_argument("--suite", choices=NOTES)
    inputs.add_argument("--remote-repo", type=Path, help="Nightly Git checkout for the checked PWM String Pad fields")
    parser.add_argument("--after-stock-pad", action="store_true",
                        help="seed the stock PAD PWM macros before the factory program transition")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    suite = args.suite or "controls"
    # Verify every selected record before compiling, writing outputs or scoring.
    release = prepare_release(args.release_ipa) if args.release_ipa else None
    settings = list(cases(args.banks, suite, args.selected_records, release)) if not args.remote_repo else []
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    reference, candidate, stock_hashes, inputs = build(args.compact, args.repo, out, args.after_stock_pad)
    rows = []
    factory_input = args.banks is not None or args.selected_records is not None or release is not None
    scope = "factory-main-pass base oscillator only" if factory_input else f"isolated oscillator {suite}"
    if args.remote_repo:
        settings = ((*row, {}) for row in remote_fields.load(args.remote_repo, out, SPEC))
        scope = "six Remote-verified PWM String Pad control words, pre-filter oscillator only"
    for uid, layer, selector, amount, enable, authored in settings:
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
                             with_peers.ctypes.data_as(F32), FRAMES, 1) or not np.array_equal(b.view(np.uint32), with_peers.view(np.uint32)):
                raise RuntimeError("another oscillator changed the selected layer's signal")
            if not reference(stop, width, increment.value, a.ctypes.data_as(F32), FRAMES):
                raise RuntimeError("reference render failed")
            if not all(np.all(np.isfinite(x)) and np.any(x) for x in (a, b)):
                raise ValueError("non-finite or silent oscillator")
            rms, spectral = channel_score(a, b, SPEC["sample_rate"], SPEC["band_hz"])
            rows.append({**authored, "case": uid, "layer": layer + 1, "note": note, "shape": stop,
                         "selector": float(selector), "amount": float(amount), "enable": float(enable),
                         "increment": increment.value, "reference_width": width,
                         "reference_sha256": digest(a.tobytes()), "candidate_sha256": digest(b.tobytes()),
                         "level_db": rms, "spectral_db": spectral,
                         "dc_difference": float(np.mean(b.astype(float) - a)),
                         "correlation": float(np.corrcoef(a, b)[0, 1]),
                         "pass": abs(rms) <= SPEC["level_limit_db"] and spectral <= SPEC["spectral_limit_db"]})
    result = {"reference_commit": REF, "reference_header_sha256": REF_HASH, "candidate_files": inputs,
              "scope": scope,
              "stock_stereo_sha256_by_mode": stock_hashes,
              "frames_per_case": FRAMES, "rate": SPEC["sample_rate"],
              "unchanged_with_other_layer_and_voice": len(rows),
              "passes": sum(row["pass"] for row in rows), "total": len(rows), "cases": rows}
    if args.after_stock_pad:
        result["stock_pad_predecessor"] = {"rate_7bit": 12, "depth_7bit": 15}
    if args.remote_repo:
        result["remote_commit"] = remote_fields.REMOTE_REV
        result["remote_input_sha256"] = remote_fields.FILES
    if factory_input:
        spec_bytes = release[0] if release else SPEC_FILE.read_bytes()
        if release:
            result["factory_input"] = "original build-9 application bundle"
            result["source_ipa_sha256"] = release[1]["ipa_sha256"]
        else:
            result["factory_input"] = "selected records" if args.selected_records is not None else "full archive"
        result["s4_inputs_sha256"] = digest(spec_bytes)
        result["record_sha256_by_uid"] = {row["program_uid"]: row["record_sha256"]
                                          for row in json.loads(spec_bytes)["programs"]}
        result["pwm_amount_stage"] = "main preset pass, before the captured-route postpass and live modulation"
    (out / "result.json").write_text(json.dumps(result, indent=2, allow_nan=False) + "\n")
    print(f"{result['passes']}/{len(rows)} isolated oscillator comparisons within limits")
    return int(result["passes"] != len(rows))


if __name__ == "__main__":
    sys.exit(main())
