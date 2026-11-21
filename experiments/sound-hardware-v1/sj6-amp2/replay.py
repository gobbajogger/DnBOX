#!/usr/bin/env python3
"""Check all original bank values through the compact Juno runtime and render each AMP2 stage."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from bank_fields import decode_factory

HERE = Path(__file__).resolve().parent

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--repo", type=Path, required=True)
    ap.add_argument("--banks-zip", type=Path, required=True)
    ap.add_argument("--gen", type=Path, help="generated stock headers; default REPO/build/gen")
    ap.add_argument("--ubsan", action="store_true")
    args = ap.parse_args()
    repo = args.repo.resolve()
    generated = (args.gen or repo / "build/gen").resolve()
    source_list = subprocess.check_output([sys.executable, "-c",
        "from tools import build; print('\\n'.join(build.ORIGINAL_SYNTH_UNITS + build.ORIGINAL_SYNTH_BACKENDS))"],
        cwd=repo,text=True).splitlines()
    with tempfile.TemporaryDirectory(prefix="sj6-amp2-") as directory:
        out = Path(directory)
        payload, factory_report = decode_factory(repo, args.banks_zip)
        (out/"values").write_bytes(payload)
        extra = ["-fsanitize=undefined", "-fno-sanitize-recover=all"] if args.ubsan else []
        subprocess.run(["cc", "-std=c99", "-O2", "-ffunction-sections", "-fdata-sections",
            "-DX0X_HOST", "-DX0X_ORIGINAL_SYNTH=1", "-DX0X_WOBBLE=0", "-DX0X_DEV_WOBBLE=0",
            "-DORIGINAL_USE_SOFT_FMA", "-DMIAMI_FX_NO_HOST_FENV", "-fno-fast-math", "-ffp-contract=off",
            "-Ifirmware/src", "-Ifirmware/src/app", "-Ifirmware/src/dsp", "-I"+str(generated),
            "-Iexperiments/original_synth", "-Iexperiments/wobble", *extra, str(HERE/"probe.c"),
            *source_list, "-Wl,--gc-sections", "-lm", "-o", str(out/"probe")], check=True, cwd=repo)
        rendered = subprocess.check_output([str(out/"probe"), str(out/"values")], text=True)
        print(json.dumps({"factory": factory_report, "runtime": json.loads(rendered)}, indent=2))

if __name__ == "__main__":
    main()
