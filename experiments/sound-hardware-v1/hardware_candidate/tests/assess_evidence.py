#!/usr/bin/env python3
"""Assess only permitted numeric behavior briefs and this candidate's files.

Never reads original engine source, source-aware adaptation reports, or audio
assets. Calibration and agreement use the same supplied numeric observations;
this is not a claim of held-out or whole-instrument equivalence.
"""
import argparse
import csv
import hashlib
import json
import math
import os
import platform
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parents[1]
BRIEF = HERE.parent / "mapping/results/implementer_brief"
OUT = HERE / "evidence"
STOCK_HASHES = {
    "juno6.c": "286f4f7392ec7250d5f46d0f542775967544b58ae9eb161caf528a25a46cb502",
    "juno6.h": "625ac8ce03b978d996ed995fda11ccd6a838f8c816ba29def8583217c17193d4",
    "fastmath.h": "fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09",
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def stock_directory(explicit):
    """Locate stock inputs after relocation; never substitute different bytes."""
    if explicit:
        choices = [Path(explicit).expanduser().resolve()]
    else:
        choices = [HERE.parent / "firmware/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/dsp"]
        # Supports experiments/<package>/hardware_candidate and an owning
        # repository's firmware/src/dsp without depending on scratch prefixes.
        choices += [parent / "firmware/src/dsp" for parent in HERE.parents]
    for candidate in choices:
        if not all((candidate / name).is_file() for name in STOCK_HASHES):
            continue
        actual = {name: sha(candidate / name) for name in STOCK_HASHES}
        mismatches = {name: {"expected": digest, "actual": actual[name]}
                      for name, digest in STOCK_HASHES.items() if actual[name] != digest}
        if mismatches:
            raise RuntimeError("Stock source hash mismatch; refusing replacement inputs at "
                               + str(candidate) + ": " + json.dumps(mismatches, sort_keys=True))
        return candidate.resolve()
    raise RuntimeError("Exact permitted stock files not found. Pass --stock-dsp PATH or "
                       "DNBX_STOCK_DSP pointing to the hash-pinned stock DSP directory.")


def write_json(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def run(args, name, env=None):
    result = subprocess.run(args, cwd=HERE, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, env=env)
    (OUT / name).write_text(result.stdout)
    if result.returncode:
        raise RuntimeError(f"{args} failed ({result.returncode}); see {name}")
    return result.stdout


def observed_fit(rows):
    # OLS y[k+1] = retention*y[k] + intercept, using only captured endpoints
    # away from stage clipping. Exact segment boundaries are numeric metadata.
    windows = [(1, "attack", 0, 480, 480), (2, "attack", 0, 1440, 1440),
               (1, "decay", 496, 2880, 2400), (2, "decay", 2400, 11984, 9600),
               (1, "release", 4096, 8192, 4800), (2, "release", 18304, 32688, 14400)]
    fit = []
    for case, stage, start, end, duration in windows:
        values = [float(r["last_observed_level"]) for r in rows
                  if int(r["case_index"]) == case and start <= int(r["block_start_frame"]) < end]
        x, y = values[:-1], values[1:]
        xm, ym = sum(x) / len(x), sum(y) / len(y)
        retention = sum((a-xm)*(b-ym) for a,b in zip(x,y)) / sum((a-xm)**2 for a in x)
        intercept = ym - retention*xm
        fit.append({"case": case, "stage": stage, "endpoint_count": len(values),
                    "retention_per_16_frames": retention,
                    "asymptote": intercept/(1-retention),
                    "normalized_exponent": -duration*math.log(retention)/16,
                    "maximum_recurrence_residual": max(abs(b-retention*a-intercept) for a,b in zip(x,y))})
    return fit


def main():
    global OUT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stock-dsp", default=os.environ.get("DNBX_STOCK_DSP"),
                        help="Explicit permitted stock directory; all three exact hashes must match")
    parser.add_argument("--output-dir", type=Path,
                        help="Write newly generated receipts to this directory instead of evidence")
    parser.add_argument("--verify-inputs-only", action="store_true",
                        help="Verify relocated stock inputs and exit without writing any receipts")
    args = parser.parse_args()
    stock = stock_directory(args.stock_dsp)
    if args.verify_inputs_only:
        print(json.dumps({"status": "VERIFIED", "stock_dsp": str(stock),
                          "sha256": STOCK_HASHES}, indent=2))
        return
    if args.output_dir:
        OUT = args.output_dir.expanduser().resolve()
    OUT.mkdir(parents=True, exist_ok=True)
    result = run(["./build/test_core", str(OUT / "candidate_amp_trace.csv")], "functional_tests.txt")
    sizes = {name: int(value) for name,value in re.findall(r"(part|params|env|voice|work)_bytes=(\d+)", result)}
    observed = list(csv.DictReader((BRIEF / "amp_observed_levels.csv").open()))
    candidate = {(int(r["case_index"]), int(r["frame"])): float(r["amp"])
                 for r in csv.DictReader((OUT / "candidate_amp_trace.csv").open())}
    agreement = []
    for case in (1, 2):
        errors = []
        for r in observed:
            if int(r["case_index"]) != case:
                continue
            for end in ("first", "last"):
                frame, level = int(r[f"{end}_observed_frame"]), float(r[f"{end}_observed_level"])
                value = candidate[case, frame]
                errors.append((abs(value-level), frame, value, level))
        worst = max(errors)
        agreement.append({"case": case, "observed_endpoint_count": len(errors),
                          "maximum_absolute_error": worst[0],
                          "rms_error": math.sqrt(sum(e[0]**2 for e in errors)/len(errors)),
                          "worst_frame": worst[1], "candidate_at_worst": worst[2],
                          "observed_at_worst": worst[3], "temporal_alignment_shift": 0})
    write_json("envelope_fit.json", {
        "scope": "generic finite-time exponential fit from permitted numeric amplitude observations",
        "fit_and_agreement_are_same_calibration_dataset": True,
        "method": "OLS on consecutive 16-frame observed endpoints, then solve finite-duration asymptote approach",
        "fits": observed_fit(observed),
        "candidate_constants": {"attack_overshoot": .2231306, "fall_overshoot": .00708345},
        "candidate_policy": ["First positive-duration attack tick flat at its endpoint",
                             "Subsequent internal ticks interpolate their endpoints, independent of caller blocks",
                             "Zero hold after a completed timed attack occupies one 16-frame tick",
                             "Zero attack and instantaneous-stage restart settle without a dead sample",
                             "Compensated tick accumulation and a stable small exponential step for long stages"],
        "agreement": agreement,
        "limits": ["Captured multiplicative amp path only; no whole-instrument audio equivalence",
                   "No active original filter-envelope curve follows from these recordings",
                   "48kHz observations do not establish original 44.1kHz behavior",
                   "New wave/filter/voice graph uses DnBOX primitives and remains separately unqualified"]})
    inputs = [stock / "juno6.c", stock / "juno6.h", stock / "fastmath.h"]
    inputs += sorted(BRIEF.glob("*"))
    write_json("source_provenance.json", {
        "permitted_inputs": [{"path": str(p), "sha256": sha(p)} for p in inputs if p.is_file()],
        "candidate_sources": [{"path": str(p.relative_to(HERE)), "sha256": sha(p)}
                              for p in sorted((HERE / "src").glob("*")) if p.is_file()],
        "stock_fastmath_vendored_unchanged": sha(stock / "fastmath.h") == sha(HERE / "src/fastmath.h"),
        "original_engine_source_read": False,
        "prior_source_adaptation_used": False,
        "production_writes": False,
        "notes": "Stock permitted breaks.c/.h absent at specified commit; no other copies read."})
    symbol_output = run(["nm", "-u", "build/dnbx_core.o"], "host_undefined_symbols.txt")
    forbidden = [name for name in ("malloc", "calloc", "realloc", "free", "pthread", "fopen", "printf")
                 if re.search(r"\b" + name + r"\b", symbol_output)]
    if forbidden:
        raise RuntimeError(f"Forbidden core dependency: {forbidden}")
    host_size = run(["size", "-A", "build/dnbx_core.o"], "host_object_sections.txt")
    stack = (HERE / "build/dnbx_core.su").read_text()
    (OUT / "host_stack_usage.txt").write_text(stack)
    write_json("size_ledger.json", {
        "host_abi_sizes": sizes,
        "raw_value_storage_bytes_in_part": 218*4 + 7*4,
        "two_part_state_bytes": sizes["part"]*2,
        "replacement_backend": {"existing_union_bytes_per_part": 3720,
                                "candidate_part_bytes": sizes["part"],
                                "existing_stock_chorus_bytes": 536,
                                "sum_if_chorus_remains_in_same_union": sizes["part"]+536,
                                "remaining_union_bytes": 3720-sizes["part"]-536},
        "baseline_given_by_owner": {"selected_FX_bytes_per_part": 4252,
                                    "pool_used_bytes": 335092, "pool_capacity_bytes": 344064,
                                    "raw_free_bytes": 8972, "required_reserve_bytes": 8192,
                                    "margin_above_reserve_bytes": 780,
                                    "margin_above_required_additional_512_bytes": 268,
                                    "XIP_free_Elite_bytes": 1180, "XIP_free_Pro_bytes": 1188,
                                    "required_XIP_floor_bytes": 1024},
        "core_has_heap_or_IO_undefined_symbols": False,
        "host_sections_text": host_size,
        "whole_product_fit_established": False,
        "notes": "Candidate state is a replacement, not an extra allocation. Host object size is not native flash size. Native replacement link, other engines/FX, stack and real callback timing remain owner's independent gates."})
    catalog = json.loads((BRIEF / "public_control_catalog.json").read_text())
    with (OUT / "raw_control_coverage.csv").open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["app_id", "identifier", "admission", "automatic_DSP_binding", "qualification"])
        for control in catalog["controls"]:
            ident = control["id"]
            policy = "reserved retained value" if ident == 207 else "external key-lock retained metadata" if 212 <= ident <= 214 else "opaque finite value retained"
            writer.writerow([ident,control["identifier"],policy,"NONE","physical setter ABI is separate; gain/selector transfer not inferred"])
    write_json("build_environment.json", {"system": platform.platform(), "machine": platform.machine(),
                                           "compiler": subprocess.check_output(["cc", "--version"],text=True).splitlines()[0],
                                           "flags": "-std=c11 -O2 -ffp-contract=off -fno-fast-math -DX0X_HOST -Wall -Wextra -Werror",
                                           "sanitizer_note": "ASan+UBSan pass with ASAN_OPTIONS=detect_leaks=0; LeakSanitizer unsupported under container ptrace. No heap references in core object."})
    print(json.dumps({"sizes": sizes, "agreement": agreement, "forbidden_core_symbols": forbidden}, indent=2))


if __name__ == "__main__":
    main()
