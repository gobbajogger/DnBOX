#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compile the real ED_INLOG dispatch arm for stock and trace-enabled SMK."""
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
INCLUDE = ROOT / "firmware/src"
editor = (INCLUDE / "app/editor.c").read_text()
arm = re.search(r"^#if[^\n]*\n +case ED_INLOG:[^\n]*\n.*?^#endif$", editor, re.M | re.S)
assert arm is not None
case_block = arm.group()
header = INCLUDE / "app/smk_inlog_profile.h"
prefix = f'#include "{header}"\n' if header.exists() else ""
source = """
#include <assert.h>
#include <stdint.h>
static unsigned recorded, replied, requested;
static unsigned note(void) { return ++recorded; }
void smk_inlog(uint32_t src, uint32_t packet) { (void)src; (void)packet; }
static void smk_inlog_reply(uint32_t arg) { requested = arg; ++replied; }
static void dispatch(unsigned cmd, const uint8_t *a, unsigned na)
{
    enum {ED_INLOG = 38};
    switch (cmd) {
""" + case_block + """
    default: return;
    }
}

int main(void)
{
    uint8_t arg = 6;
#ifdef SMK_INLOG_RECORD
    SMK_INLOG_RECORD(note(), 0);
    SMK_INLOG_RECORD(note(), 0);
#endif
    dispatch(38, &arg, 1);
    assert(requested == (EXPECT_REPLIES ? 6 : 0));
    dispatch(38, &arg, 0);
    assert(replied == EXPECT_REPLIES);
    assert(requested == 0);
    assert(recorded == EXPECT_REPLIES);
    return 0;
}
"""
with tempfile.TemporaryDirectory(prefix="smk-inlog-profile-") as tmp:
    c, binary = Path(tmp) / "test.c", Path(tmp) / "test"
    c.write_text(prefix + source)
    for trace, replies in ((0, 0), (1, 2)):
        subprocess.run([os.environ.get("CC", "cc"), "-std=c99", "-DBOARD_SMK37=1",
                        f"-DX0X_LCD_TRACE={trace}", f"-DEXPECT_REPLIES={replies}",
                        str(c), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
print("stock INLOG: no unsupported reply or recorder work; trace INLOG remains available")
