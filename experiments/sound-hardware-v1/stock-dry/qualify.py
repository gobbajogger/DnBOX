#!/usr/bin/env python3
"""Compare the actual stock dry voice to 60 saved and 24 source-rendered original scenes."""
import argparse
import contextlib
import hashlib
import io
import json
import os
from pathlib import Path
import runpy
import shutil
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
METRIC_SHA = "bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1"
PINNED_BASE = "b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2"
PINNED_ACTIVE = "3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(test, message):
    if not test:
        raise ValueError(message)


def build_comparator(seed, out):
    for name, sha in json.loads((HERE / "comparison-inputs.json").read_text()).items():
        require(digest(seed / name) == sha, f"Comparison input differs: {name}")
    wrapper = out / "comparison.cpp"
    shutil.copyfile(seed / "capture_original_dry.cpp", wrapper)
    subprocess.run(["patch", "--batch", "--fuzz=0", wrapper, HERE / "comparison-wrapper.patch"], check=True)
    closure = seed / "staged_reference/closure"
    sp = closure / "experiment/audiokit-4.9.2/AudioKit/Core"
    includes = [closure / d for d in ("scheduler", "experiment/probe", "experiment/shared-graph", "controls")]
    includes += [sp / d for d in ("Soundpipe", "SoundpipeExtension", "Soundpipe/lib/dr_wav")]
    obj = out / "comparison.o"
    compiler = os.environ.get("CXX", "c++")
    subprocess.run([compiler, "-std=c++17", "-O2", "-fno-builtin", "-ffunction-sections",
                    "-fdata-sections", "-ffp-contract=off", "-fno-fast-math", "-DNO_LIBSNDFILE",
                    *[f"-I{d}" for d in includes], "-c", wrapper, "-o", obj], check=True)
    objects = json.loads((HERE / "comparison-inputs.json").read_text())
    binary = out / "original"
    subprocess.run([compiler, obj, *[seed / n for n in objects if n.endswith(".o")],
                    "-Wl,--gc-sections", "-lm", "-o", binary], check=True)
    return binary


def original_capture(binary, inputs, controls, dest, case):
    command = [binary, inputs / "public-morphs.f32", controls, inputs / "tuning128.f32le", dest, case]
    receipt = json.loads(subprocess.check_output(command))
    require(receipt["retained_frames"] == 264576 and receipt["warmup_nonfinite_count"] == 0
            and receipt["output_nonfinite_count"] == 0, f"Invalid original capture: {case}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("recovery", type=Path, help="Extracted DnBOX-Sound-final-recovery root")
    parser.add_argument("output", type=Path, help="New output directory")
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir()
    seed = args.recovery.resolve() / "comparison_only/reference_filter_inputs/full_dry_preparation"
    inputs = seed / "inputs"
    originals = args.recovery.resolve() / "reference/dry_reference_packet"
    metric = originals / "metrics.py"
    require(digest(metric) == METRIC_SHA, "Measurement script differs")
    comp = build_comparator(seed, out)
    for name, expected in [("baseline", PINNED_BASE), ("active", PINNED_ACTIVE)]:
        dest = out / (name + ".control.pcm")
        original_capture(comp, inputs, inputs / (name + "-controls.f32le"), dest, name)
        require(digest(dest) == expected, "Original control replay differs: " + name)

    dsp = REPO / "firmware/src/dsp"
    binary = out / "voice"
    subprocess.run([os.environ.get("CC", "cc"), "-std=c11", "-O2", "-ffp-contract=off", "-fno-fast-math",
                    f"-I{dsp}", HERE / "render.c", dsp / "s1_dry_voice.c", dsp / "dnbx_s1_envelope.c",
                    "-o", binary], check=True)
    reports = []
    for case in json.loads((originals / "case_plan.json").read_text())["cases"]:
        stem = case["case_id"].replace("/", "-")
        dynamic = case["case_id"].startswith("filter-modulation/")
        if dynamic:
            controls = np.fromfile(inputs / "active-controls.f32le", "<f4")
            controls[[16, 17, 19]] = [case["cutoff_hz"], case["resonance"], case["filter_depth"]]
            command_data = out / (stem + ".controls")
            controls.tofile(command_data)
            ref = out / (stem + ".original")
            original_capture(comp, inputs, command_data, ref, case["case_id"])
        else:
            ref = originals / case["current_reference_mono_relative_path"]
            require(digest(ref) == case["current_reference_mono_sha256"], "Saved reference differs: " + stem)
        kind = "dynamic" if dynamic else "static"
        cand = out / (stem + ".candidate")
        subprocess.run([binary, cand, *[str(case[k]) for k in ("velocity", "cutoff_hz", "resonance", "filter_depth")],
                        kind], check=True)
        require(ref.stat().st_size == cand.stat().st_size == 1058304, "Incomplete audio: " + stem)
        paired = out / (stem + ".paired")
        np.column_stack([np.fromfile(f, "<f4") for f in (ref, cand)]).astype("<f4").tofile(paired)
        sys.argv = [str(metric), str(paired), "1"]
        with contextlib.redirect_stdout(io.StringIO()):
            runpy.run_path(str(metric), run_name="__main__")
        record = json.loads(paired.with_suffix(".json").read_text())
        require(record["gain"] == 1 and [r["note"] for r in record["rows"]] == [36, 60, 84],
                "Incomplete or adjusted comparison: " + stem)
        reports.append(dict(case=case["case_id"], kind=kind, reference=digest(ref), candidate=digest(cand), **record))

    summary = {}
    for kind, expected_count in [("static", 180), ("dynamic", 72)]:
        selected = [s for s in reports if s["kind"] == kind]
        rows = [r for s in selected for r in s["rows"]]
        require(len(rows) == expected_count, "Missing " + kind + " notes")
        summary[kind] = dict(passed=sum(r["rms_db"] <= 1 and r["spectral_db"] <= 3 for r in rows),
                             total=len(rows), max_rms_db=max(r["rms_db"] for r in rows),
                             max_spectral_db=max(r["spectral_db"] for r in rows),
                             nonfinite=sum(s["nonfinite"] for s in selected))
    print(json.dumps(summary, indent=2))
    (out / "results.json").write_text(json.dumps(dict(summary=summary, cases=reports), indent=2) + "\n")
    return int(any(v["passed"] != v["total"] or v["nonfinite"] for v in summary.values()))


if __name__ == "__main__":
    sys.exit(main())
