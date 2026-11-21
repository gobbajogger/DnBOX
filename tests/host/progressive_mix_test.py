#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Check wet stereo output across progressive rendering and CPU2 takeover."""
import importlib.util
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


def main():
    root = Path(__file__).resolve().parents[2]
    spec = importlib.util.spec_from_file_location("x0x_build", root / "tools/build.py")
    build = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(build)
    subprocess.run(["sh", "host/build_host.sh", "--generate-only"], cwd=root, check=True)

    with tempfile.TemporaryDirectory() as directory:
        tmp = Path(directory)
        engine = root / "tests/host/progressive_mix_engine.c"
        cc = shlex.split(os.environ.get("CC", "cc"))
        flags = [
            "-O1", "-std=c11", "-pthread", "-ffp-contract=off", "-fno-fast-math",
            "-D_POSIX_C_SOURCE=200809L", "-D_DARWIN_C_SOURCE", "-DX0X_HOST",
            "-DX0X_ORIGINAL_SYNTH=1", "-DX0X_WOBBLE=0", "-DX0X_DEV_WOBBLE=1",
            "-DORIGINAL_USE_SOFT_FMA", "-DMIAMI_FX_NO_HOST_FENV",
            "-I.", "-Ifirmware/src", "-Ifirmware/src/app", "-Ifirmware/src/dsp",
            "-Iexperiments/original_synth", "-Iexperiments/wobble", "-Ibuild/gen",
        ]
        obj = tmp / "engine.o"
        units = [f"firmware/src/{p}" for p in build.O2_UNITS if p != "app/engine.c"]
        units += ["firmware/src/app/factory_banks.c", *build.ORIGINAL_SYNTH_UNITS,
                  *build.ORIGINAL_SYNTH_BACKENDS, "tests/host/progressive_mix_test.c"]
        executable = tmp / "mix"
        link = ["-no-pie"] if sys.platform.startswith("linux") else []
        objects = []
        for index, unit in enumerate(units):
            target = tmp / f"unit-{index}.o"
            subprocess.run(cc + flags + ["-c", unit, "-o", target], cwd=root, check=True)
            objects.append(target)

        def build_engine(plan_flags):
            subprocess.run(cc + flags + plan_flags + [
                "-DX0X_HOST_ASYNC_PREVIEW", "-Dplat_cpu2_run=mix_test_submit",
                "-Dplat_cpu2_wait=mix_test_join", "-c", engine, "-o", obj,
            ], cwd=root, check=True)
            subprocess.run(cc + flags + link + [*objects, obj, "-lm", "-o", executable],
                           cwd=root, check=True)

        build_engine(["-DX0X_CORE_PLAN=31"])

        def capture(mode, scenario=1):
            pcm = tmp / "output.pcm"
            subprocess.run([executable, str(mode), str(int(scenario)), pcm], check=True)
            return pcm.read_bytes()

        serial = capture(0)
        if not any(serial):
            raise AssertionError("serial mix is silent")
        for mode in range(1, 5):
            if capture(mode) != serial:
                raise AssertionError(f"worker mode {mode} changed the serial mix")
        if capture(3, False) == serial:
            raise AssertionError("disabling BREAK hits left the mix unchanged")
        reference = capture(0, 2)
        for mode in range(1, 5):
            if capture(mode, 2) != reference:
                raise AssertionError(f"worker mode {mode} changed short-prefix PCM or state")
        print("PASS short-prefix master preview, final PUMP/motion edits and partial handoff")
        print("PASS progressive mix, takeover, wet sends and adopted synth owners")

        build_engine([])
        serial = capture(0, 3)
        for mode in (1, 2, 3):
            if capture(mode, 3) != serial:
                raise AssertionError(f"worker mode {mode} changed the attack-aware mix")
        print("PASS attack-aware planner, repeated hits, owner eligibility and ordered PCM")


if __name__ == "__main__":
    main()
