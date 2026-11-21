#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Exercise native recovery and calibration control flow against a RAM-only board."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tarfile
import tempfile

from PIL import Image

from run import exact_function, hook_once

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "firmware/src"
HAL = ROOT / "firmware/hal"
SCREENS = ROOT / "docs/lanes/fwui/screens"
SIGNATURES = (
    "static void hexs(char *b, uint32_t v)",
    "static void fm1_fault(const fm1_crash_t *c)",
    "static void enter_uboot(const char *why)",
    "static void safe_main(void)",
)
ARCHIVED = {
    "boot-default", "boot-safe-mode", "boot-safe-crash-core-0", "boot-safe-crash-core-1",
    "boot-post-crash-core-0", "boot-post-crash-core-1", "boot-safe-retry", "boot-uboot", "boot-uboot-usb", "boot-crash-diagnostics",
    *(f"hardware-calibration-button-{i:02d}" for i in range(14)),
    *(f"hardware-calibration-encoder-{i:02d}" for i in range(7)),
}
CLEAR = {
    "calibration-held-timeout-clear", "calibration-button-timeout-clear",
    "calibration-encoder-timeout-clear", "calibration-saved-clear",
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def compile_fixture(tmp):
    src = (SRC / "app/main_fm1.c").read_text()
    (tmp / "route_bodies.inc").write_text("\n".join(exact_function(src, sig) for sig in SIGNATURES))
    startup = exact_function(src, "static void fm1_main(void)")
    cut = "    x0x_dbg.boots++;"
    if startup.count(cut) != 1:
        raise ValueError("ambiguous normal startup endpoint")
    startup = startup[:startup.index(cut)] + "    stop_before_audio_startup();\n}\n"
    startup += exact_function(src, "void fm1_cstart(void)")
    (tmp / "startup_bodies.inc").write_text(startup)
    irq = (HAL / "fm1_irq.h").read_text()
    start = irq.index("#define FM1_CRASH_MAGIC ")
    definitions = irq[start:irq.index("} fm1_crash_t;", start)] + "} fm1_crash_t;\n"
    for path, names in (("smk_input.h", ("FM1_NBTN", "FM1_NENC", "SMK_ENC_MASTER")),
                        ("fm1_time.h", ("FM1_TICKS_PER_US",))):
        source = (HAL / path).read_text()
        for name in names:
            definitions += re.search(rf"^#define {name} .+$", source, re.M).group() + "\n"
    for macro in ("BOOTGUARD_MAGIC", "DBG_MAGIC"):
        definitions += re.search(rf"^#define {macro} .+$", src, re.M).group() + "\n"
    for path, pattern in ((HAL/"fm1_guard.h", r"^enum \{ FM1_GUARD_STACK[^}]+};"),
                          (HAL/"fm1_flash_map.h", r"^#define FL_JEDEC_OK[^\n]+0xEB6014u[^\n]+"),
                          (SRC/"app/slots.c", r"^#define SMP_USER_SLOTS 3$")):
        definitions += re.search(pattern, path.read_text(), re.M).group() + "\n"
    (tmp / "route_types.inc").write_text(definitions)
    subprocess.run([sys.executable, str(ROOT / "tools/gen_font.py"), str(tmp / "felucca_font.h")],
                   env=dict(os.environ, X0X_FONTSET="barlow", X0X_FONT_DIRS=""), check=True)
    build_native(tmp)


def build_native(tmp):
    subprocess.run([os.environ.get("CC", "cc"), "-std=c11", "-O1", "-DX0X_HOST=1", "-DBOARD_SMK37=1",
                    "-Werror=implicit-function-declaration", "-I", str(tmp), "-I", str(SRC),
                    str(Path(__file__).with_name("native_routes.c")), "-o", str(tmp / "check")], check=True)


def board_image(path):
    # The mock SPI endpoint stores native big-endian RGB565, exactly as sent.
    data = path.read_bytes()
    if len(data) != 240*240*2:
        raise ValueError(f"partial frame {path}")
    rgb = bytearray()
    for i in range(0, len(data), 2):
        pixel = data[i]*256 + data[i+1]
        rgb.extend(((pixel >> 11)*255//31, ((pixel >> 5) & 63)*255//63, (pixel & 31)*255//31))
    return Image.frombytes("RGB", (240, 240), bytes(rgb))


def reference_images():
    with (SCREENS / "screen-index.tsv").open() as f:
        rows = [r for r in csv.DictReader(f, delimiter="\t") if r["state_id"] in ARCHIVED]
    if len(rows) != len(ARCHIVED):
        raise ValueError("missing or duplicate archived state")
    images = {}
    for archive in {r["archive"] for r in rows}:
        with tarfile.open(SCREENS / archive) as tar:
            for r in (r for r in rows if r["archive"] == archive):
                image = Image.open(tar.extractfile(r["png_path"])).convert("RGB")
                if image.size != (240,240) or sha(image.tobytes()) != r["rgb_sha256"]:
                    raise ValueError(f"archive digest {r['state_id']}")
                images[r["state_id"]] = image
    return images


def compare(tmp, output):
    reference = reference_images()
    bg = reference["boot-safe-mode"].getpixel((0,0))
    clear = Image.new("RGB", (240,240), bg)
    for name in CLEAR:
        reference[name] = clear
    reference["lcd-init-clear"] = Image.new("RGB", (240,240), (0,0,0))
    title = clear.copy()
    title.paste(reference["hardware-calibration-button-00"].crop((0,0,240,64)))
    reference["calibration-title"] = title
    retry = reference["boot-safe-mode"].copy()
    retry.paste(reference["boot-safe-retry"].crop((0,165,240,197)), (0,165))
    reference["safe-retry-without-crash"] = retry
    paths = {p.stem:p for p in tmp.glob("*.rgb565")}
    if paths.keys() != reference.keys():
        raise ValueError("unexpected state inventory: " + str(paths.keys() ^ reference.keys()))
    result = {}
    for name, path in sorted(paths.items()):
        img = board_image(path)
        if img.tobytes() != reference[name].tobytes():
            raise ValueError(f"native frame or reference geometry changed: {name}")
        result[name] = sha(img.tobytes())
        if name not in ARCHIVED:
            img.save(output / (name + ".png"), optimize=True)
    return result


def wave_fallback(tmp):
    units = [SRC / "app/original_sound.c", *(ROOT / "experiments/original_synth" / f
             for f in ("miami_program.c", "chicago_program.c", "fused_reference.c"))]
    subprocess.run([os.environ.get("CC", "cc"), "-std=c11", "-DX0X_HOST=1", "-O2", "-ffp-contract=off",
                    "-fno-fast-math", "-I", str(SRC / "app"), "-I", str(ROOT / "experiments/original_synth"),
                    str(Path(__file__).with_name("wave_fallback.c")), *map(str,units), "-lm", "-o",
                    str(tmp / "wave")], check=True)
    result = subprocess.check_output([str(tmp / "wave")], text=True, timeout=120)
    factories, previews = map(int, result.split())
    if not factories or not previews:
        raise ValueError("wave fallback scan selected no inputs")
    return dict(validFactoryCells=factories, singleParameterPreviews=previews,
                rejectedValidInputs=0, rejectedInvalidControls=factories,
                scope="All Chicago/Miami factory cells and individual admitted non-performance wave previews; combinations untested.")


def shifted_retry_control(tmp):
    bodies = tmp / "route_bodies.inc"
    before = 'draw_text_box(0, 172, 240, &FONT_S, "STARTING DnBOX..."'
    bodies.write_text(hook_once(bodies.read_text(), before, before.replace("172", "171")))
    build_native(tmp)
    mutated = tmp / "mutated"
    mutated.mkdir()
    subprocess.run([str(tmp / "check")], cwd=mutated, check=True, timeout=30)
    try:
        compare(mutated, mutated)
    except ValueError as exc:
        if str(exc) == "native frame or reference geometry changed: boot-safe-retry":
            return "FAIL detected at boot-safe-retry after shifting the retry caption one pixel"
        raise
    raise AssertionError("shifted retry caption was accepted")


def main(output):
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as temporary:
        tmp = Path(temporary)
        compile_fixture(tmp)
        subprocess.run([str(tmp / "check")], cwd=tmp, check=True, timeout=30)
        frames = compare(tmp, output)
        waveform = wave_fallback(tmp)
        negative = shifted_retry_control(tmp)
    inputs = ["firmware/src/lcd.c", "firmware/src/gfx.c", "firmware/src/app/panel.c",
              "firmware/src/app/main_fm1.c", "firmware/hal/fm1_irq.h", "firmware/hal/smk_input.h",
              "firmware/hal/fm1_time.h", "firmware/src/app/plat.h", "firmware/src/app/boot_screen.h",
              "firmware/src/ui_theme.h", "tools/gen_font.py", "tests/ui_inventory/native_routes.py",
              "tests/ui_inventory/native_routes.c", "tests/ui_inventory/wave_fallback.c",
              "firmware/src/app/original_sound.c", "firmware/src/app/ui_original_controls.h",
              "firmware/src/app/ui_original_wave.h", "firmware/hal/fm1_guard.h", "firmware/hal/fm1_flash_map.h",
              "firmware/src/app/slots.c", "tests/ui_inventory/native_startup.inc"]
    inputs += [str(p.relative_to(ROOT)) for p in sorted((ROOT / "assets/fonts").glob("Barlow*.ttf"))]
    report = dict(status="PASS", evidenceKind="Complete production wrappers with modeled board endpoints",
                  startup="Complete fm1_cstart control flow with RAM linker regions, through the fm1_main RAM-only prefix to the end of its crash notice, or the complete safe-mode branch.",
                  functions=["fm1_cstart", "lcd_init", "panel_setup", "safe_main", "fm1_fault", "enter_uboot"],
                  geometry="All pixels equal independently archived frames or their explicitly composed references.",
                  waveFallback=waveform, geometryNegativeControl=negative, archivedFrames=len(ARCHIVED), additionalFrames=len(frames)-len(ARCHIVED),
                  rgbSha256=frames, inputSha256={p:sha((ROOT/p).read_bytes()) for p in inputs})
    (output / "report.json").write_text(json.dumps(report, indent=2)+"\n")
    print(f"PASS {len(frames)} actual 240x240 host frames, {len(ARCHIVED)} exact archive matches")


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("output", type=Path)
    main(ap.parse_args().output)
