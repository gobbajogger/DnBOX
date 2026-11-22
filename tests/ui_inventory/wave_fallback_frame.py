#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Render a rejected, out-of-range sibling lock through the true Chicago page."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from PIL import Image, ImageChops

from run import hook_once

ROOT = Path(__file__).resolve().parents[2]


def page_difference(directory):
    def load(name):
        with Image.open(directory / (name + ".png")) as image:
            return image.convert("RGB")
    diff = ImageChops.difference(load("chicago-wave-valid-lock"), load("chicago-wave-invalid-lock"))
    # V_PART original-wave cell, and the first value in its FILTER 1 row.
    # The oscillator and the deliberately changed sibling are the only owners.
    boxes = {"oscillatorPlot": (38,47,69,70), "siblingCutoffValue": (53,94,82,122)}
    diff_counts = {}
    for name, box in boxes.items():
        count = sum(n for n, color in diff.crop(box).getcolors(240*240) if color != (0,0,0))
        if not count:
            raise ValueError("missing expected difference at "+name)
        diff_counts[name] = count
        diff.paste((0,0,0), box)
    if diff.getbbox():
        raise ValueError("pixels changed outside oscillator plot and sibling cutoff value")
    return diff_counts


def main(output):
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as temporary:
        tmp = Path(temporary)
        # The retained host wrapper generates build files in its checkout.
        # Keep them private while another lane is building or testing.
        for name in ("firmware","host","tools","tests","assets","experiments"):
            shutil.copytree(ROOT/name, tmp/name, symlinks=True)
        ui = tmp / "firmware/src/app/ui.c"
        anchor = "static UI_DRAW void draw_wave_glyph(int cx, int y, int shape, int mix, uint16_t col, uint16_t face)\n{"
        ui_source_sha = hashlib.sha256(ui.read_bytes()).hexdigest()
        ui.write_text(hook_once(ui.read_text(), anchor, "static unsigned wave_probe_calls, wave_probe_hide;\n"
                                + anchor + "\n    ++wave_probe_calls; if (wave_probe_hide) return;"))
        host = tmp/"results"
        env=dict(os.environ, SMC_UI_OUT=str(host), SMC_UI_TEST_FILE="tests/ui_inventory/wave_fallback_frame.c")
        run=subprocess.run(["sh","tests/host/smc_ui_test.sh"], cwd=tmp, env=env, check=False,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
        if run.returncode:
            raise RuntimeError(run.stdout[-4000:])
        lines = [line for line in run.stdout.splitlines() if line.startswith("WAVE_FALLBACK ")]
        if len(lines) != 1:
            raise ValueError("missing or duplicate fallback pixel result")
        count, *bounds = map(int, lines[0].split()[1:])
        rows = {}
        for name in ("chicago-wave-valid-lock", "chicago-wave-invalid-lock"):
            with Image.open(host/(name+".png")) as im:
                if im.size != (240,240):
                    raise ValueError(name)
                rgb=im.convert("RGB")
                rows[name]=hashlib.sha256(rgb.tobytes()).hexdigest()
                rgb.save(output/(name+".png"), optimize=True)
    report=dict(status="PASS", source="firmware/src/app/ui.c::draw_wave_glyph",
                scope="Normal Chicago page with an intentionally invalid sibling lock injected into RAM; admitted input and hardware behavior are outside the malformed case.",
                input="Held step zero, original Chicago factory zero, cutoff lock 127/128. WAVE remains at its factory value.",
                fallbackOnlyPixels=count, boundsInclusive=bounds, rgbSha256=rows,
                validToInvalidChangedPixels=page_difference(output), outsideOwnerChanges=0,
                uiSourceSha256=ui_source_sha)
    (output/"wave-frame.json").write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report))


if __name__ == "__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("output",type=Path)
    main(ap.parse_args().output)
