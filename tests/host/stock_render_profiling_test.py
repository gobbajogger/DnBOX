#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Exercise the actual render profiling hooks in both stock and trace profiles."""
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2] / "firmware" / "src" / "app"
source = (ROOT / "engine.c").read_text()
section = re.search(r"/\* time per stage of the render.*?(?=/\* -+ state)", source, re.S)
assert section is not None
profile = ROOT / "ui_perf_profile.h"
header = f'#include "{profile}"\n' if profile.exists() else ""
exercise = """
#include <assert.h>
#include <stdint.h>
enum { ENG_PROF_N = 1 };
static uint32_t clock_reads;
static uint32_t plat_cycles(void) { return ++clock_reads; }
""" + header + section.group() + """
int main(void) {
    PROF_T(start);
    PROF_ADD(0, start);
#ifdef PROF_FRAMES
    PROF_FRAMES(256);
#endif
    assert(clock_reads == EXPECT_CLOCK_READS);
#if defined(X0X_ENGINE_PROFILE) && X0X_ENGINE_PROFILE
    assert(eng_prof[0] == 1 && eng_prof_frames == 256);
#endif
    return 0;
}
"""

with tempfile.TemporaryDirectory(prefix="render-profiling-") as tmp:
    c, binary = Path(tmp) / "test.c", Path(tmp) / "test"
    c.write_text(exercise)
    for profile_flags, clock_reads in (([], 0), (["-DX0X_LCD_TRACE=1"], 2), (["-DX0X_HOST"], 2)):
        subprocess.run([os.environ.get("CC", "cc"), "-std=c99", "-DBOARD_SMK37=1",
                        *profile_flags, f"-DEXPECT_CLOCK_READS={clock_reads}",
                        str(c), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
print("render profiling: stock never reads diagnostic cycle counters; trace does")
