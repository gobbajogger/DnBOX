#!/usr/bin/env python3
"""Compare two checked source revisions using an isolated PI32v2 Juno renderer."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BASE = "6c91d26ecb621b023334900c09e3705d2a91d86e"
CANDIDATE = "49662e4d437421e686a6cc04b806cd0d700bab74"
DSP = "firmware/src/dsp"


def run(compiler, emulator, directory):
    flags = ["-target", "pi32v2", "-mcpu=r3", "-mfprev1", "-ffp-contract=off", "-fno-fast-math",
             "-Os", "-ffunction-sections", "-fdata-sections", "-fno-builtin",
             "-I", ROOT / DSP, "-idirafter", compiler / "include"]
    clang = compiler / "bin/clang"

    def compile_object(source, name, *options):
        subprocess.run([clang, *flags, *options, "-c", source, "-o", name], check=True, cwd=directory)

    compile_object(HERE / "entry.S", "entry.o")
    rows = {}
    for ref in [BASE, CANDIDATE]:
        source = directory / "juno6.c"
        source.write_bytes(subprocess.check_output(["git", "show", ref + ":" + DSP + "/juno6.c"], cwd=ROOT))
        compile_object(source, "juno.o")
        rows[ref] = {}
        for voices in (0, 1, 2, 6):
            compile_object(HERE / "render.c", "driver.o", f"-DVOICES={voices}")
            subprocess.run([compiler / "bin/ld", "--gc-sections", "-e", "_start", "-T", HERE / "memory.ld",
                            "-o", "render.elf", "entry.o", "driver.o", "juno.o"], check=True, cwd=directory)
            subprocess.run([compiler / "bin/objcopy", "-j", ".text", "-O", "binary", "render.elf", "render.bin"],
                           check=True, cwd=directory)
            output = subprocess.check_output([emulator, "--bin", "render.bin", "--entry", "0x02000000",
                                              "--steps", "3000000", "--dump", "1c07f00", "1"],
                                             cwd=directory, text=True)
            if "halt: core 0 jumped to 0x00000000" not in output:
                raise ValueError("test did not return through the entry stub")
            count = int(re.search(r"stopped after (\d+) instructions", output)[1])
            checksum = re.search(r"^  0x01c07f00: (\w{8})$", output, re.MULTILINE)[1]
            rows[ref][str(voices)] = {"instructions": count, "final_stereo_checksum": checksum}
    return rows


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("toolchain", type=Path, help="JieLi root containing pi32v2")
    parser.add_argument("emulator", type=Path, help="fm1-emu executable")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as temp:
        os.environ["TMPDIR"] = temp
        result = run(args.toolchain.resolve() / "pi32v2", args.emulator.resolve(), Path(temp))
    print(json.dumps(result, indent=2))
    raise SystemExit(result != json.loads((HERE / "results.json").read_text()))
