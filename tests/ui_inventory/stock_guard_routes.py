#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run the unmodified target through two exception vectors, safe mode and PLAY."""
import argparse
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

from PIL import Image

from native_routes import board_image, compile_fixture
from stock_boot_fixture import ROOT, digest, load_symbols, require
from stock_panel_routes import LUT_5, LUT_6

KEYS = "15200:SEQPLAY,15600:SEQPLAY^"
FRAME_BYTES = 240 * 240 * 3


def capture(args, folder, name, ms, symbols, oracle):
    case = folder / name
    (case / "work").mkdir(parents=True)
    frames = case / "frames"
    frames.mkdir()
    env = {k:v for k,v in os.environ.items() if not k.startswith("FM1_")}
    env.update(FM1_STRICT="1", FM1_DNBOX_GUARD_TEST="1", FM1_RESET_TRACE="1",
               FM1_BENCH_KEYS=KEYS, FM1_DUMPRAM=str(case / "ram.bin"))
    cmd = [args.emulator, "--fwsc", args.fwsc, "--mhz", "360", "--lcd-only",
           "--shot", str(ms), "--flash", case / "flash.bin", "--record", "-"]
    log_path = args.output / (name + ".log")
    first = {}
    count = 0
    previous = None
    with log_path.open("wb") as log, subprocess.Popen(cmd, cwd=case, env=env,
            stdout=subprocess.PIPE, stderr=log) as process:
        with process.stdout as stream:
            while header := stream.readline():
                require(header == b"P6\n" and stream.readline() == b"240 240\n" and
                        stream.readline() == b"255\n", "invalid LCD movie header")
                pixels = stream.read(FRAME_BYTES)
                require(len(pixels) == FRAME_BYTES, "incomplete LCD movie frame")
                if pixels != previous:
                    panel = Image.frombytes("RGB", (240,240), pixels)
                    canonical = panel.point(LUT_5 + LUT_6 + LUT_5)
                    sha = digest(canonical.tobytes())
                    if sha not in first:
                        first[sha] = 16 * count
                        panel.save(frames / (sha + ".png"))
                previous = pixels
                count += 1
        require(process.wait() == 0, "target did not finish the requested shot")
    log = log_path.read_text()
    ram = (case / "ram.bin").read_bytes()
    def target_object(symbol):
        offset, length = symbols[symbol]
        require(0 <= offset <= len(ram) - length, symbol)
        return ram[offset:offset+length]
    raw = target_object("fm1_crash")
    crash = struct.unpack("<17I", raw)
    debug = struct.unpack("<13I", target_object("x0x_dbg"))
    guard = struct.unpack("<3I", target_object("bootguard"))
    require((crash[0],crash[2],crash[6],crash[16]) == (0x43525348,1,16,0),
            "target did not enter core 0's modeled guard exception vector")
    (case / "crash.bin").write_bytes(raw)
    subprocess.run([oracle], cwd=case, check=True)
    matches = {}
    for label in ("fault","safe","retry","post-crash"):
        expected = board_image(case / (label + ".rgb565"))
        sha = digest(expected.tobytes())
        if sha in first:
            matches[label] = {"firstMs":first[sha], "canonicalRgbSha256":sha}
            (args.output / (name + "-" + label + ".png")).write_bytes((frames / (sha + ".png")).read_bytes())
    assert "fault" in matches
    resets = [int(s) for s in re.findall(r"chip reset at (\d+) ms \(PR_PWR\)", log)]
    if name == "first":
        require(crash[1] == 1 and not resets and debug[10] == 1, "first diagnostic record")
    else:
        require(crash[1] == 2 and debug[10] == 3 and debug[7] > 0 and debug[12] == 2,
                "post-retry ordinary boot or crash readback")
        require(set(matches) == {"fault","safe","retry","post-crash"}, "missing complete recovery frames")
        require(len(resets) == 3 and resets[0] >= 5200 and resets[1] >= 13200 and
                resets[1] < matches["safe"]["firstMs"] < matches["retry"]["firstMs"] < resets[2] <
                matches["post-crash"]["firstMs"], "fault, holdoff, retry and reboot order")
    return dict(lcdFrames=count, crashRecord=list(crash), debug=list(debug), bootguard=list(guard),
                firstMatchingFrames=matches, resetsMs=resets)


def run(args):
    for name in ("emulator","fwsc","elf","output"):
        setattr(args, name, getattr(args,name).resolve())
    args.output.mkdir(parents=True, exist_ok=False)
    symbols = load_symbols(args.elf)
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        compile_fixture(folder)
        oracle = folder / "guard-oracle"
        here = Path(__file__).resolve().parent
        subprocess.run([os.environ.get("CC","cc"), "-std=c11", "-O1", "-DX0X_HOST=1", "-DBOARD_SMK37=1",
                        "-Werror=implicit-function-declaration", "-I", str(folder), "-I", str(ROOT/"firmware/src"),
                        str(here/"stock_guard_oracle.c"), "-o", str(oracle)], check=True)
        receipt = dict(model=args.model, fwscSha256=digest(args.fwsc.read_bytes()),
                       elfSha256=digest(args.elf.read_bytes()), emulatorSha256=digest(args.emulator.read_bytes()),
                       guardMs=[1200,9200], panelKeys=KEYS, shots={})
        for name, ms in (("first",1900), ("safe-play",20000)):
            receipt["shots"][name] = capture(args, folder, name, ms, symbols, oracle)
    (args.output/"guard-routes.json").write_text(json.dumps(receipt,indent=2)+"\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", choices=("elite","pro"), required=True)
    for name in ("emulator","fwsc","elf"):
        parser.add_argument("--"+name, type=Path, required=True)
    parser.add_argument("output", type=Path)
    run(parser.parse_args())
