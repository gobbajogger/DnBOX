#!/usr/bin/env python3
"""Exercise the production callback and parameter panel with every worker mask."""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("root", type=Path)
    ap.add_argument("engine", type=Path)
    ap.add_argument("generated_headers", type=Path)
    ap.add_argument("out", type=Path)
    args = ap.parse_args()
    root, engine, generated, out = [x.resolve() for x in (args.root, args.engine, args.generated_headers, args.out)]
    out.mkdir(parents=True, exist_ok=True)
    source = Path(__file__).with_name("panel.c").resolve()
    masks = [0, 1, 4, 5, 8, 9, 12, 13, 16, 21, 25, 29, 31]
    forced = out / "forced-mask.h"
    forced.write_text("extern unsigned test_worker_mask;\n#define X0X_CORE_PLAN test_worker_mask\n")
    original = subprocess.check_output(["sh", "-c",
        'set -eu; . tests/host/original_build.sh; echo "$ORIGINAL_FLAGS"; echo "$ORIGINAL_SOURCES"'],
        cwd=root, text=True).splitlines()
    flags = ["cc", "-O1", "-ffp-contract=off", "-std=c99", "-D_POSIX_C_SOURCE=200809L", "-DX0X_HOST",
             *shlex.split(original[0]), "-I.", "-Ifirmware/src", "-Ifirmware/src/app",
             "-Ifirmware/src/dsp", "-I" + str(generated), "-include", str(forced)]
    sources = [engine if f == "firmware/src/app/engine.c" else root/f for f in shlex.split(original[1])]
    objects = out / "objects"
    objects.mkdir(exist_ok=True)
    def compile_unit(path):
        dst = objects / (hashlib.sha1(path.read_bytes()).hexdigest() + "-" + path.stem + ".o")
        if not dst.exists():
            tmp = dst.with_suffix(".partial")
            subprocess.run([*flags, "-c", path, "-o", tmp], check=True, cwd=root)
            tmp.replace(dst)
        return dst
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        compiled = list(pool.map(compile_unit, [source, *sources]))
    binary = out / "scheduler-trace"
    subprocess.run(["cc", *compiled, "-lm", "-o", binary], check=True)
    results = {}
    for pair in (1, 3):
        traces = {}
        for route in range(3):
            digests = []
            sample = out / "trace.bin"
            for mask in masks:
                subprocess.run([binary, str(pair), str(mask), str(route), sample], check=True)
                digests.append(hashlib.sha256(sample.read_bytes()).hexdigest())
            if len(set(digests)) != 1:
                raise AssertionError((pair, route, dict(zip(masks, digests))))
            traces[str(route)] = digests[0]
            print(f"pair={pair} route={route} masks={len(masks)} sha256={digests[0]}", flush=True)
        if traces["1"] != traces["2"] or traces["0"] == traces["1"]:
            raise AssertionError(("control path", traces))
        results[str(pair)] = traces
    (out / "results.json").write_text(json.dumps(dict(masks=masks, results=results), indent=2) + "\n")
if __name__ == "__main__":
    main()
