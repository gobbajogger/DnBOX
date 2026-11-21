#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run BREAK recovery on a linked SMK package with fm1-live and private flash files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image

from run import exact_function

ROOT = Path(__file__).resolve().parents[2]
EXPECTED_SPLASH = ROOT / "docs/lanes/fwui/current/native-routes/pending-shadow/post-recovery-splash.png"


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


def run(args):
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    fwsc, emulator = args.fwsc.resolve(), args.emulator.resolve()
    evidence = {"packageSha256": digest(fwsc.read_bytes()), "emulatorSha256": digest(emulator.read_bytes()),
                "model": args.model, "strict": True, "clockMHz": 360, "reportedTimeMs": args.ms}
    with tempfile.TemporaryDirectory() as temp:
        tmp = Path(temp)
        (tmp / "work").mkdir()
        size, owner, boot, live, shadow, splash, good, stale = seed_sectors(tmp)
        flash = tmp / "flash.bin"
        env = {k:v for k,v in os.environ.items() if not k.startswith("FM1_")}
        env.update(FM1_STRICT="1", FM1_BENCH_KEYS="")

        def boot_to(ms, log):
            with (out / log).open("w") as stream:
                subprocess.run([emulator, "--fwsc", fwsc, "--mhz", "360", "--lcd-only",
                                "--shot", str(ms), "--flash", flash], cwd=tmp, env=env,
                               stdout=stream, stderr=subprocess.STDOUT, check=True, timeout=180)

        boot_to(1, "seed.log")
        image = bytearray(flash.read_bytes())
        require(len(image) == 0x100000 and "flash: 0 erases / programs" in (out/"seed.log").read_text(),
                "package flash seed changed")
        app_hash = digest(image[:0x9D000])
        image[owner:owner+16] = b"SMK37DATAX0X" + bytes(4)
        image[boot:boot+size] = splash
        erased = bytes([255]) * size
        image[live:live+size] = erased if args.case == "recover" else good
        image[shadow:shadow+size] = good if args.case == "recover" else stale
        require(digest(image[:0x9D000]) == app_hash, "fixture overwrote package/app")
        flash.write_bytes(image)
        env["FM1_DUMPRAM"] = str(tmp / "ram.bin")
        boot_to(args.ms, "stock.log")
        final = flash.read_bytes()
        require(digest(final[:0x9D000]) == app_hash, "package/app changed in emulator")
        require(final[live:live+size] == good, "valid sample missing")
        require(final[shadow:shadow+size] == erased, "sample shadow remains")
        with Image.open(tmp / "work/lcd.ppm") as img:
            require(img.size == (240,240), "unexpected LCD size")
            actual = img.convert("RGB")
        actual.save(out / "screen.png", optimize=True)
        evidence.update(case=args.case, seedFlashSha256=digest(image), finalFlashSha256=digest(final),
                        sampleSha256=digest(good), screenRgbSha256=digest(actual.tobytes()))
        if args.ms == 448:
            with Image.open(EXPECTED_SPLASH) as img:
                host = img.convert("RGB")
            # The host multiplies by 255 and truncates. fm1-live copies high bits to the low end.
            # Both preserve the 5/6/5 source in their upper channel bits.
            for i, (x, y) in enumerate(zip(actual.getdata(), host.getdata())):
                require(all((a >> s) == (b >> s) for a,b,s in zip(x,y,(3,2,3))),
                        "wrong splash pixel at " + str(divmod(i,240)))
            evidence["matchingSplashPixelsRGB565"] = 240*240
    (out/"stock.json").write_text(json.dumps(evidence, indent=2)+"\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", choices=("elite","pro"), required=True)
    parser.add_argument("--emulator", type=Path, required=True)
    parser.add_argument("--fwsc", type=Path, required=True)
    parser.add_argument("--case", choices=("recover","already-live"), required=True)
    parser.add_argument("--ms", type=int, default=448)
    parser.add_argument("output", type=Path)
    run(parser.parse_args())
