#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run BREAK recovery on a linked SMK package with fm1-live and private flash files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

from PIL import Image

from run import exact_function

ROOT = Path(__file__).resolve().parents[2]
EXPECTED_SPLASH = ROOT / "docs/lanes/fwui/current/native-routes/pending-shadow/post-recovery-splash.png"
EXPECTED_ORDINARY = ROOT / "docs/lanes/fwui/current/native-routes/normal-loop/startup-store-owned.png"
RAM_BASE = 0x01C00000


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def seed_sectors(tmp):
    # Use the exact seven-case C fixture and production slot ABI/CRC.
    source = (ROOT / "tests/ui_inventory/startup_loop.c").read_text()
    storage = (ROOT / "firmware/src/storage.c").read_text()
    definitions = "\n".join(line for line in storage.splitlines()
                            if line.startswith(("#define ST_SECTOR ", "#define ST_OWNER_SECTOR ")))
    program = '''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BOARD_SMK37 1
#define CHECK(x) do { if (!(x)) abort(); } while (0)
static uint8_t flash_memory[0x100000];
#define SMP_USER_XIP(k) (flash_memory + SMP_USER_OFF(k))
#include "app/slots.c"
#include "app/boot_screen.h"
#define st_crc32 x0x_crc32
''' + definitions + "\n" + exact_function(source, "static void seed_splash(")
    program += exact_function(source, "static void sample_sector(") + r'''
int main(void) {
    uint8_t good[ST_SECTOR], stale[ST_SECTOR];
    memset(flash_memory, 0xff, sizeof flash_memory);
    seed_splash(0);
    sample_sector(good, 0x36);
    sample_sector(stale, 0x59);
    printf("%u %u %u %u %u\n", ST_SECTOR, ST_OWNER_SECTOR, BOOT_SEC0,
           SMP_USER_OFF(0), SMP_SEC0_BACKUP_OFF(0));
    fwrite(flash_memory + BOOT_SEC0, ST_SECTOR, 1, stdout);
    fwrite(good, ST_SECTOR, 1, stdout);
    fwrite(stale, ST_SECTOR, 1, stdout);
    return ferror(stdout) ? 1 : 0;
}
'''
    binary = tmp / "seed"
    subprocess.run(["cc", "-std=c11", "-O2", "-I", str(ROOT / "firmware/src"),
                    "-x", "c", "-", "-o", binary], input=program, text=True, check=True)
    layout, data = subprocess.check_output([binary]).split(b"\n", 1)
    size, owner, boot, live, shadow = map(int, layout.split())
    require(len(data) == 3 * size, "short sample fixture")
    return size, owner, boot, live, shadow, data[:size], data[size:2*size], data[2*size:]


def load_symbols(elf):
    result = subprocess.check_output(["nm", "-S", elf], text=True)
    return {p[3]: (int(p[0], 16) - RAM_BASE, int(p[1], 16))
            for line in result.splitlines() if len(p := line.split()) == 4}


def rgb565(image):
    # Both RGB exporters preserve the upper 5/6/5 bits. Their low-bit expansion differs.
    rgb = image.convert("RGB").tobytes()
    return b"".join(struct.pack(">H", (r >> 3) << 11 | (g >> 2) << 5 | b >> 3)
                    for r, g, b in zip(rgb[::3], rgb[1::3], rgb[2::3]))


def run(args):
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    emulator, fwsc, elf = (p.resolve() for p in (args.emulator, args.fwsc, args.elf))
    symbols = load_symbols(elf)
    with Image.open(EXPECTED_SPLASH) as img:
        splash565 = rgb565(img)
    with Image.open(EXPECTED_ORDINARY) as img:
        ordinary565 = rgb565(img)
    evidence = {"model": args.model, "emulatorSha256": digest(emulator.read_bytes()),
                "fwscSha256": digest(fwsc.read_bytes()), "elfSha256": digest(elf.read_bytes()),
                "clockMHz": 360, "strict": True, "cases": {}}

    with tempfile.TemporaryDirectory() as dirname:
        tmp = Path(dirname)
        size, owner, boot, live, shadow, splash, good, stale = seed_sectors(tmp)
        erased = bytes([255]) * size

        def shot(name, ms, flash=None):
            case = tmp / name
            (case / "work").mkdir(parents=True)
            flashfile = case / "flash.bin"
            if flash is not None:
                flashfile.write_bytes(flash)
            env = {k: v for k, v in os.environ.items() if not k.startswith("FM1_")}
            env.update(FM1_STRICT="1", FM1_BENCH_KEYS="", FM1_DUMPRAM=str(case / "ram.bin"))
            with (output / (name + ".log")).open("w") as log:
                subprocess.run([emulator, "--fwsc", fwsc, "--mhz", "360", "--shot", str(ms),
                                "--lcd-only", "--flash", flashfile], cwd=case, env=env,
                               stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
            log = (output / (name + ".log")).read_text()
            ops = re.findall(r"^flash: (\d+) erases / programs$", log, re.M)
            require(len(ops) == 1, "missing stock flash counter for " + name)
            with Image.open(case / "work/lcd.ppm") as img:
                require(img.size == (240, 240), "unexpected LCD size")
                rgb = img.convert("RGB")
            rgb.save(output / (name + ".png"), optimize=True)
            stored = flashfile.read_bytes()
            evidence["cases"][name] = {"ms": ms, "flashOperations": int(ops[0]),
                                       "finalFlashSha256": digest(stored), "rgbSha256": digest(rgb.tobytes()),
                                       "rgb565Sha256": digest(rgb565(rgb))}
            if flash is not None:
                evidence["cases"][name]["inputFlashSha256"] = digest(flash)
            return stored, (case / "ram.bin").read_bytes(), rgb565(rgb)

        base, _, _ = shot("package", 1)
        require(len(base) == 0x100000 and evidence["cases"]["package"]["flashOperations"] == 0,
                "package seed is not a clean 1 MiB flash image")
        base = bytearray(base)
        base[owner:owner + 16] = b"SMK37DATAX0X" + bytes(4)
        base[boot:boot + size] = splash

        def checkpoint(name, initial):
            require(initial[:0x9D000] == base[:0x9D000], "package/app changed before boot")
            final, ram, pixels = shot(name, 1500, initial)
            require(final[live:live + size] == good, "wrong recovered sample")
            require(final[shadow:shadow + size] == erased, "shadow not cleared")
            expected = bytearray(initial)
            expected[live:live + size] = good
            expected[shadow:shadow + size] = erased
            require(final == expected, name + " changed bytes outside live/shadow sectors")
            require(pixels == ordinary565, name + " differs from the native host ordinary screen")
            data = {}
            for symbol, length in (("x0x_dbg", 52), ("usr_nz", 3), ("nloops", 4)):
                offset, actual = symbols[symbol]
                require(actual == length and 0 <= offset <= len(ram)-actual, symbol)
                data[symbol] = ram[offset:offset+actual]
            dbg = struct.unpack("<13I", data["x0x_dbg"])
            count = struct.unpack("<I", data["nloops"])[0]
            require(dbg[0] == 0x44424731 and dbg[1] and dbg[6] and dbg[7] and
                    dbg[10] == 1 and not dbg[12], "did not reach ordinary UI/IRQ/audio")
            require(data["usr_nz"][0] == 1 and count > 0, "restored loop not published")
            evidence["cases"][name].update(audioHalves=dbg[1], timerIRQs=dbg[6], uiFrames=dbg[7],
                                            boots=dbg[10], crashSeen=dbg[12], slotZeroZones=data["usr_nz"][0],
                                            loopCount=count)
            return final, pixels

        recovered = None
        for name, slot, backup in (("recover", erased, good), ("already-live", good, stale)):
            initial = bytearray(base)
            initial[live:live + size] = slot
            initial[shadow:shadow + size] = backup
            _, _, pixels = shot(name + "-splash", 448, initial)
            require(pixels == splash565, name + " splash differs in panel RGB565")
            restored, pixels = checkpoint(name, initial)
            operations = evidence["cases"][name]["flashOperations"]
            require(operations == (18 if name == "recover" else 1), name + " flash operation count")
            if name == "recover":
                recovered = restored
            else:
                require(restored == recovered, "control and recovery flash differ")
        persisted, pixels = checkpoint("restart", recovered)
        require(persisted == recovered, "recovered flash cold restart differs")
        require(evidence["cases"]["restart"]["flashOperations"] == 0,
                "cold restart rewrote recovered flash")
    evidence["hostSplashMatchingPixels"] = 240 * 240
    evidence["hostOrdinaryMatchingPixels"] = 240 * 240
    (output / "stock-boot.json").write_text(json.dumps(evidence, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", choices=("elite", "pro"), required=True)
    parser.add_argument("--emulator", type=Path, required=True)
    parser.add_argument("--fwsc", type=Path, required=True)
    parser.add_argument("--elf", type=Path, required=True)
    parser.add_argument("output", type=Path)
    run(parser.parse_args())

