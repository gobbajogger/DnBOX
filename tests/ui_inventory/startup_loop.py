#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compile the full normal SMK startup and exercise its framebuffer notices."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from PIL import Image

from run import exact_function

ROOT = Path(__file__).resolve().parents[2]


def headers(root, out):
    src = root/"firmware/src"
    hal = root/"firmware/hal"
    main = (src/"app/main_fm1.c").read_text()
    storage = (src/"storage.c").read_text()
    editor = (src/"app/editor.c").read_text()
    ota = (src/"ota.c").read_text()
    constants = []
    for source, prefixes in ((main,["#define DBG_MAGIC "]), (storage,["#define ST_SECTOR ","#define ST_SMK_BASE ","#define ST_OWNER_SECTOR "]),
                             ((hal/"smk_input.h").read_text(),["enum { FM1_BTN_OCT_DOWN","enum { FM1_ADC_BATT","#define FM1_OCT_BOTH ","#define SMK_TRIM_FLASH "]),
                             ((hal/"fm1_irq.h").read_text(),["#define FM1_CRASH_MAGIC "]),
                             (ota,["#define OTA_AREA ","#define OTA_AREA_LEN ","#define OTA_RES "]),
                             ((hal/"fm1_flash_map.h").read_text(),["#define FL_JEDEC_OK("])):
        for prefix in prefixes:
            constants.append(next(line for line in source.splitlines() if line.startswith(prefix)))
    constants.append("static uint8_t st_owner_cleared;")
    out.mkdir(parents=True,exist_ok=True)
    (out/"normal_constants.inc").write_text("\n".join(constants)+"\n")
    backup = editor.split("#if SMP_SEC0_BACKUP",1)[1].split("#else",1)[0]
    bodies = [exact_function(storage,"static void st_owner_check(void)"),
              exact_function(editor,"static uint32_t ed_smp_slot(uint32_t k)"),
              exact_function(backup,"static int ed_smp_copy_4k(uint32_t dst, const uint8_t *src)"),
              exact_function(backup,"static int ed_smp_backup_present(uint32_t k)"),
              exact_function(backup,"static void ed_smp_clear_backup(uint32_t k)"),
              exact_function(backup,"static int ed_smp_restore_sec0(uint32_t k)"),
              exact_function(backup,"static void ed_smp_try_restore(uint32_t k)"),
              exact_function(backup,"void ed_smp_recover(void)"),
              exact_function(ota,"static void ota_unstage(void)"),
              exact_function(ota,"static int ota_boot_cleanup(void)"),
              exact_function(editor,"static const uint8_t *boot_shown(void)"),
              exact_function(main,"static void hexs(char *b, uint32_t v)"),
              exact_function(main,"static void fm1_main(void)")]
    (out/"normal_bodies.inc").write_text("\n".join(bodies))


SPLASH_IDS = {"boot-default", "boot-custom-text", "boot-custom-picture"}
INPUTS = (
    "firmware/src/app/main_fm1.c", "firmware/src/app/ui.c", "firmware/src/app/engine.c",
    "firmware/src/app/editor.c", "firmware/src/app/boot_screen.h", "firmware/src/storage.c",
    "firmware/src/ota.c", "firmware/hal/smk_input.h", "firmware/hal/fm1_irq.h",
    "firmware/hal/fm1_flash_map.h", "tests/ui_inventory/startup_loop.c",
    "tests/ui_inventory/startup_loop.py", "tests/ui_inventory/run.py",
    "tests/host/smc_ui_test.c", "tests/host/smc_ui_test.sh",
)


def main(output):
    output.mkdir(parents=True, exist_ok=True)
    with (ROOT/"docs/lanes/fwui/screens/screen-index.tsv").open() as index:
        splashes = {row["state_id"]: row for row in csv.DictReader(index, delimiter="\t")
                    if row["state_id"] in SPLASH_IDS}
    with tempfile.TemporaryDirectory() as temporary:
        tmp = Path(temporary)
        for name in ("firmware","host","tools","tests","assets","experiments"):
            shutil.copytree(ROOT/name,tmp/name,symlinks=True)
        host=tmp/"result"
        headers(tmp,host)
        source_hashes = {name: hashlib.sha256((tmp/name).read_bytes()).hexdigest() for name in INPUTS}
        generated_hashes = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                            for path in sorted(host.glob("normal_*.inc"))}
        masks={}
        digests={}
        case_frames={}
        for mode in ("normal","owned","picture","hold","last-release"):
            case=tmp/mode
            case.mkdir()
            env=dict(os.environ,SMC_UI_OUT=str(host), SMC_UI_TEST_FILE="tests/ui_inventory/startup_loop.c",
                     STARTUP_CASE=mode, SMC_UI_SHOTS=str(case))
            if mode=="normal":
                # The retained wrapper sets SMC_UI_SHOTS to its output dir.
                command=["sh","tests/host/smc_ui_test.sh"]
                case=host
            else:
                command=[str(host/"test")]
            run=subprocess.run(command,env=env,cwd=tmp,capture_output=True,text=True,timeout=360)
            if run.returncode or "PASS complete fm1_main host path" not in run.stdout:
                raise RuntimeError((run.stdout+run.stderr)[-6000:])
            for name,n in re.findall(r"^CAPTION (\S+) (\d+)$",run.stdout,re.M):
                if masks.setdefault(name,int(n)) != int(n):
                    raise ValueError(f"inconsistent caption mask: {name}")
            case_frames[mode] = sorted(path.stem for path in case.glob("*.png"))
            for path in case.glob("*.png"):
                with Image.open(path) as image:
                    rgb=image.convert("RGB")
                    assert rgb.size==(240,240)
                    digest=hashlib.sha256(rgb.tobytes()).hexdigest()
                    if digests.setdefault(path.stem,digest) != digest:
                        raise ValueError(f"inconsistent frame: {path.stem}")
                    if path.stem in SPLASH_IDS:
                        if digest != splashes[path.stem]["rgb_sha256"]:
                            raise ValueError(f"archived splash differs: {path.stem}")
                    else:
                        rgb.save(output/path.name,optimize=True)
    expected={"startup-store-cleared","update-hold-3","update-hold-2","update-hold-1",
              "update-hold-0","update-hold-cancelled","update-last-tick-release"}
    if set(masks)!=expected or set(digests)!=expected|{"startup-store-owned","startup-notice-expired","update-cancel-expired","boot-default","boot-custom-text","boot-custom-picture"}:
        raise ValueError("incomplete normal startup coverage")
    index_file = output/"screen-index.tsv"
    with index_file.open("w", newline="") as dest:
        writer = csv.writer(dest, delimiter="\t", lineterminator="\n")
        writer.writerow(["state_id","scope","file","png_path_in_archive","rgb_sha256"])
        for state, digest in sorted(digests.items()):
            row = splashes.get(state)
            filename = "../../../screens/"+row["archive"] if row else state+".png"
            writer.writerow([state, "normal-fm1_main/host-hardware", filename,
                             row["png_path"] if row else "", digest])
    report = dict(
        status="PASS", registeredTest="ui-startup-loop", screenIndex="screen-index.tsv",
        sourceSha256=source_hashes, extractedInputSha256=generated_hashes,
        stateIds=len(digests), newPngs=len(digests)-len(SPLASH_IDS),
        archivedSplashComparisons=len(SPLASH_IDS), frameSize=[240,240],
        caseFrames=case_frames, captionInkPixels=masks, captionInkTotal=sum(masks.values()),
        rgbSha256=digests,
        bounds="All seven final caption masks match the native font, including antialiasing, within x3..213 and above the lower edge of the native header.",
        cases={
            "normal": "Foreign owner with custom data, no pending sample shadow and stale OTA staging. Native owner reset restores the factory splash. Five OTA sectors clear. OLD DATA CLEARED expires. Releasing both octaves after the 1.. frame requests UPDATE MODE CANCELLED. Ordinary octave input replaces it with STEPS 17-32 before rendering.",
            "owned": "Matching owner, custom text splash, first ordinary frame without a clear notice.",
            "picture": "Matching owner, custom picture splash, first ordinary frame without a clear notice.",
            "hold": "Both octaves stay held. At 5000 ms a due frame displays UPDATE MODE IN 0... At 5001 ms the native loop calls enter_uboot with UBOOT.",
            "last-release": "At exactly 5000 ms the frame displays UPDATE MODE IN 0.. and shown becomes zero. Both buttons release at 5001 ms. The firmware skips the cancellation notice and renders STEPS 17-32 at the next due frame."
        },
        scope="Complete production fm1_main on the SMK host with accepted emulated JEDEC, RAM storage, empty sample shadows, simulated board interfaces and scripted clocks and buttons. Pending sample recovery, physical drivers, IRQ/audio timing and actual bootloader execution remain outside this capture."
    )
    (output/"normal-loop.json").write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report))


if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("output",type=Path)
    main(ap.parse_args().output)
