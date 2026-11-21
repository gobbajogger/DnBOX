#!/usr/bin/env python3
"""Prepare and replay the exact native replacement experiment in a fresh folder.

This creates no Git branch and never touches a device. The existing repository
supplies the immutable consolidated source. SDK/toolchain inputs are explicit.
"""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile

HERE = Path(__file__).resolve().parent
BASE_SHA = "76040daa5b52d686a8b7b544296ba5d79610a95d"
UNIT = "dnbx_core_7161c21e6d9d773c"

def run(argv, cwd=None, env=None):
    subprocess.run([str(x) for x in argv], cwd=cwd, env=env, check=True)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--toolchain", type=Path, required=True)
    ap.add_argument("--sdk", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--prepare-only", action="store_true")
    args = ap.parse_args()
    tc, sdk, out = (p.expanduser().resolve() for p in
                    (args.toolchain, args.sdk, args.output))
    if not (tc / "pi32v2/bin/clang").is_file():
        ap.error("toolchain must contain pi32v2/bin/clang")
    if not (sdk / "cpu/wl82/tools/uboot.boot").is_file():
        ap.error("SDK must contain cpu/wl82/tools/uboot.boot")
    if out.exists():
        ap.error("output must be a new directory; preserved receipts are not overwritten")
    repo = Path(subprocess.check_output(
        ["git", "-C", str(HERE), "rev-parse", "--show-toplevel"], text=True).strip())
    subprocess.run(["git", "-C", str(repo), "cat-file", "-e", BASE_SHA + "^{tree}"],
                   check=True)
    out.mkdir(parents=True)
    resources = out / "resource_review"
    baseline = resources / "native-baseline-76040"
    baseline.mkdir(parents=True)
    # Git emits repository paths; explicitly reject unsafe archive members.
    with tempfile.TemporaryFile() as archive:
        subprocess.run(["git", "-C", str(repo), "archive", "--format=tar", BASE_SHA],
                       stdout=archive, check=True)
        archive.seek(0)
        with tarfile.open(fileobj=archive) as entries:
            for member in entries.getmembers():
                name = Path(member.name)
                if name.is_absolute() or ".." in name.parts:
                    raise RuntimeError("unsafe archived path")
            entries.extractall(baseline, filter="data")
    source = HERE / "resource_review" / UNIT
    unit = resources / UNIT
    unit.mkdir()
    for name in ("prepare_replacement.py", "replacement_native.py",
                 "compact_variant_native.py", "replacement_smoke.c",
                 "CANDIDATE-PROVENANCE.json"):
        shutil.copyfile(source / name, unit / name)
    shutil.copytree(source / "src", unit / "src")
    compact = HERE / "hardware_candidate/size_variants/compact_profile"
    shutil.copytree(compact, out / "hardware_candidate/size_variants/compact_profile",
                    ignore=shutil.ignore_patterns("build", "*.o", "*.so", "__pycache__"))
    inputs = out / "build_inputs/extracted"
    inputs.mkdir(parents=True)
    (inputs / "jieli-linux-toolchains-20260730.1").symlink_to(tc, target_is_directory=True)
    (inputs / "fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13").symlink_to(
        sdk, target_is_directory=True)
    pins = {
        "juno6.c": "286f4f7392ec7250d5f46d0f542775967544b58ae9eb161caf528a25a46cb502",
        "juno6.h": "625ac8ce03b978d996ed995fda11ccd6a838f8c816ba29def8583217c17193d4",
        "fastmath.h": "fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09",
    }
    for name, expected in pins.items():
        actual = hashlib.sha256((baseline / "firmware/src/dsp" / name).read_bytes()).hexdigest()
        if actual != expected:
            raise RuntimeError("baseline input hash mismatch: " + name)
    print("Prepared exact baseline and isolated source at", out, flush=True)
    if args.prepare_only:
        return
    env = dict(os.environ, JIELI_TOOLCHAIN=str(tc), AC79_SDK=str(sdk),
               X0X_GEN_CACHE=str(resources / "gen-cache"))
    for model in ("elite", "pro"):
        run(["python3", "tools/build.py", "--board", "smk37", "--model", model],
            cwd=baseline, env=env)
    run(["python3", unit / "prepare_replacement.py"], env=env)
    run(["python3", unit / "replacement_native.py"], env=env)
    run(["python3", unit / "compact_variant_native.py"], env=env)
    print("Read physical-link return codes separately from diagnostic overflow images.")

if __name__ == "__main__":
    main()
