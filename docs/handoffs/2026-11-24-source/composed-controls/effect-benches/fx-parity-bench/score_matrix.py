#!/usr/bin/env python3
"""Score independently rendered FX cases without bundling source audio."""
from argparse import ArgumentParser
from hashlib import sha256
import json
from pathlib import Path
import subprocess
import sys

ARMS = ("original-dry", "original-wet", "compact-dry", "compact-wet")
FORMATS = {"wav", "f32le", "i16le"}

def digest(path):
    with path.open("rb") as handle:
        return sha256(handle.read()).hexdigest()

def score(manifest, results, require_threshold_pass=False):
    matrix = json.loads(manifest.read_text())
    if matrix.get("version") != 1 or not matrix.get("cases"):
        raise ValueError("expected a nonempty, version 1 FX capture matrix")
    if results.exists():
        raise ValueError("choose a new output directory")
    results.mkdir(parents=True)
    report = []
    used = set()
    for case in matrix["cases"]:
        name = case["name"]
        if not name or not all(c.isascii() and (c.isalnum() or c in "-_") for c in name) or name in used:
            raise ValueError(f"invalid or duplicate case name: {name!r}")
        used.add(name)
        provenance = case["provenance"]
        if any(not provenance.get(k) for k in ("original", "compact", "stimulus", "effect")):
            raise ValueError(f"{name}: identify both sources, stimulus and effect")
        spec = case.get("format", "wav")
        if spec not in FORMATS:
            raise ValueError(f"{name}: unknown sample format")
        opts = ["--format", spec]
        specific = case.get("arm-formats", {})
        if any(arm not in ARMS or fmt not in FORMATS for arm, fmt in specific.items()):
            raise ValueError(f"{name}: unknown arm or capture format")
        for arm, fmt in specific.items():
            opts += ["--" + arm + "-format", fmt]
        if any(specific.get(arm, spec) != "wav" for arm in ARMS):
            rate = case.get("rate")
            if type(rate) is not int or rate <= 0:
                raise ValueError(f"{name}: raw captures need explicit sample rate")
            opts += ["--rate", str(rate)]
        if "compact-leading-samples" in case:
            if not case.get("alignment-reason"):
                raise ValueError(f"{name}: a supplied alignment needs a reason")
            opts += ["--compact-leading-samples", str(case["compact-leading-samples"])]
        for key in ("window", "tail", "max-dry-null-dbfs", "max-effect-null-dbfs", "max-tail-null-dbfs", "envelope-ms"):
            if key in case:
                opts += ["--" + key, str(case[key])]
        inputs = {}
        for arm in ARMS:
            path = (manifest.parent / case["captures"][arm]).resolve(strict=True)
            if not path.is_file():
                raise ValueError(f"{name}: capture is not a file: {arm}")
            inputs[arm] = digest(path)
            opts += ["--" + arm, str(path)]
        dest = results / (name + ".json")
        proc = subprocess.run([sys.executable, str(Path(__file__).with_name("compare.py")),
                               *opts, "--output", str(dest)], check=False)
        entry = {"name": name, "provenance": provenance, "format": spec,
                 "sha256": inputs, "report": dest.name if dest.is_file() else None,
                 "comparator_exit": proc.returncode}
        for key in ("rate", "window", "tail", "arm-formats", "compact-leading-samples", "alignment-reason"):
            if key in case:
                entry[key] = case[key]
        suites = ("dry", "effect_wet_minus_dry", "tail")
        if dest.is_file():
            metrics = json.loads(dest.read_text())
            thresholds = metrics.get("caller_thresholds", {})
            entry["threshold_status"] = thresholds.get("status", "not_reported")
            checks = thresholds.get("checks", {})
            entry["threshold_cases_complete"] = all(
                checks.get(name, {}).get("status") == "pass" for name in suites)
        else:
            entry["threshold_status"] = "invalid"
            entry["threshold_cases_complete"] = False
        report.append(entry)
        # Save each completed case even if the next one fails to render or parse.
        (results / "index.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    return int(any(
        row["comparator_exit"] or (require_threshold_pass and
        (row["threshold_status"] != "pass" or not row["threshold_cases_complete"]))
        for row in report))

def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("matrix", type=Path)
    parser.add_argument("output_directory", type=Path)
    parser.add_argument("--require-threshold-pass", action="store_true",
                        help="require explicit passing dry, FX and tail null bounds in every case")
    args = parser.parse_args()
    return score(args.matrix.resolve(strict=True), args.output_directory,
                 args.require_threshold_pass)

if __name__ == "__main__":
    raise SystemExit(main())
