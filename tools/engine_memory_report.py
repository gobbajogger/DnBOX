#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Per-engine memory report for one linked SMK-37 app (Elite or Pro).

  tools/engine_memory_report.py <x0x.elf> [--model elite|pro] [--nm <nm>]

Reads the ELF symbol table (JieLi nm, `nm -S`) and attributes every sized symbol to an
engine by its name, and to a memory area by its address:
  flash  XIP app code area  [APP_XIP, APP_XIP + APP_CODE_MAX)
  ram    ordinary RAM       .data + .bss, [0x01C08000, 0x01C20000)
  pool   the pool           [_pool_start, _pool_end)
  xdata  SMK XDATA region   [XDATA_XIP, XDATA_XIP + XDATA_SIZE): flash 0xAD000, font bitmaps outside the app
  other  core-1 stack, xpool, noinit, BLE RAM, RAM code
Symbols that match no engine are "shared" (UI, sequencer, platform, shared DSP). The engine
counts are lower bounds: inlined code and merged literals stay in the caller's symbol.

The report ends with the stock budget lines and exits 1 if
  XIP free < XIP_MIN (1024 B) or pool margin < POOL_MIN (512 B),
the same limits the gate's budget step uses, or if the XDATA region (16 B header + blob) overflows its 8 KiB. Last line: "engine-memory: PASS|FAIL <model>".
"""
import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

APP_XIP = 0x02000120
APP_CODE_MAX = 0x86EE0           # tools/fm1pkg_make.py APP_CODE_MAX (FL_BLE_LO - 0x4120)
RAM_LO, RAM_HI = 0x01C08000, 0x01C20000
RAM_SIZE = 98304                 # tools/build.py check(): ".data+.bss ... of 98304"
POOL_SIZE = 0x54000              # tools/build.py: pool raw free = 0x54000 - pool
POOL_RESERVE = 8192
XDATA_XIP, XDATA_SIZE = 0x020A9000, 0x2000   # fm1_flash_map.h FL_XDATA_LO 0xAD000 via FM1_XIP; app.ld XDATA
XIP_MIN = int(os.environ.get("XIP_MIN", "1024"))
POOL_MIN = int(os.environ.get("POOL_MIN", "512"))

# First match wins. Order matters: specific engine prefixes before generic ones.
ENGINES = [
    ("synthone", r"^(s1_|dnbx_s1|synthone|S1_)"),
    ("superj6", r"^(juno|dnbx_core|dnbx_|sj6|superj6|jn_|JUNO)"),
    ("chicago", r"^(original_chicago|chicago|ck_|CK_)"),
    ("miami", r"^(original_miami|miami|mi_)"),
    ("909", r"^(d9_|drum909|d909|process\.909|cy_|x0x_smp_|engine_bd909|bd909)"),
    ("808", r"^(d8_|drum808|d808|eng_808|process\.808)"),
    ("break", r"^(brk|breaks|engine_brk|break)"),
    ("bass", r"^(engine_bass|bass_|tb3po|eng_bass)"),
    ("fx", r"^(fx|bfx|fxbus|master|dly_buf|original_fx|original_master|eng_fx|miami_fx)"),
    ("original-runtime", r"^(original_)"),
]
_ENGINE_RE = [(n, re.compile(p)) for n, p in ENGINES]


def engine_of(name):
    for n, rx in _ENGINE_RE:
        if rx.search(name):
            return n
    return "shared"


def parse_nm(text):
    """nm -S lines: 'addr size type name' -> list of (addr, size, type, name)."""
    syms, marks = [], {}
    for line in text.splitlines():
        p = line.split(None, 3)
        if len(p) == 4 and re.fullmatch(r"[0-9a-fA-F]+", p[0]) and re.fullmatch(r"[0-9a-fA-F]+", p[1]):
            syms.append((int(p[0], 16), int(p[1], 16), p[2], p[3]))
        elif len(p) == 3 and re.fullmatch(r"[0-9a-fA-F]+", p[0]):
            marks[p[2]] = int(p[0], 16)
    return syms, marks


def area_of(addr, pool_lo, pool_hi):
    if APP_XIP <= addr < APP_XIP + APP_CODE_MAX:
        return "flash"
    if XDATA_XIP <= addr < XDATA_XIP + XDATA_SIZE:
        return "xdata"
    if RAM_LO <= addr < RAM_HI:
        return "ram"
    if pool_lo <= addr < pool_hi:
        return "pool"
    return "other"


def report(syms, marks, image_bytes=None):
    pool_lo, pool_hi = marks.get("_pool_start"), marks.get("_pool_end")
    if pool_lo is None or pool_hi is None:
        raise ValueError("missing _pool_start/_pool_end")
    table = {}
    # Aliases and overlapping symbols (unions, local labels) share bytes: count each byte once,
    # for the first symbol (lowest address, then largest) that covers it.
    covered = 0
    for addr, size, _typ, name in sorted(syms, key=lambda s: (s[0], -s[1], s[3])):
        lo, hi = max(addr, covered), addr + size
        if size == 0 or hi <= lo or name.startswith(".L"):   # .L_MergedGlobals: a container
            continue
        covered = hi
        row = table.setdefault(engine_of(name), {"flash": 0, "ram": 0, "pool": 0, "other": 0, "xdata": 0})
        row[area_of(addr, pool_lo, pool_hi)] += hi - lo
    pool_used = pool_hi - pool_lo
    bss_end = marks.get("_bss_end")
    ram_used = (bss_end - RAM_LO) if bss_end is not None else None
    if image_bytes is None:
        image_bytes = sum(r["flash"] for r in table.values())
    xs, xe = marks.get("_xdata_start"), marks.get("_xdata_end")
    xdata_used = (xe - xs) if xs is not None and xe is not None else sum(r.get("xdata", 0) for r in table.values())
    return table, {
        "xdata_used": xdata_used,
        "xdata_free": XDATA_SIZE - xdata_used,
        "xip_free": APP_CODE_MAX - image_bytes,
        "pool_used": pool_used,
        "pool_margin": POOL_SIZE - pool_used - POOL_RESERVE,
        "ram_used": ram_used,
        "ram_free": (RAM_SIZE - ram_used) if ram_used is not None else None,
    }


def render(model, table, totals):
    out = [f"engine memory ({model}): bytes by engine (lower bounds; shared = UI, sequencer, platform, shared DSP)",
           f"  {'engine':<18}{'flash':>9}{'ram':>9}{'pool':>9}{'other':>9}"]
    order = [n for n, _ in ENGINES] + ["shared"]
    for n in order:
        r = table.get(n)
        if r:
            out.append(f"  {n:<18}{r['flash']:>9}{r['ram']:>9}{r['pool']:>9}{r['other']:>9}")
    out.append(f"XIP free: {totals['xip_free']} B (min {XIP_MIN})")
    out.append(f"pool margin: {totals['pool_margin']} B (min {POOL_MIN}; pool {totals['pool_used']} of {POOL_SIZE}, reserve {POOL_RESERVE})")
    if totals["ram_used"] is not None:
        out.append(f"ordinary RAM: {totals['ram_used']} of {RAM_SIZE} B, {totals['ram_free']} B free")
    if totals.get("xdata_used"):
        out.append(f"XDATA: {totals['xdata_used']} of {XDATA_SIZE} B (flash 0xAD000), {totals['xdata_free']} B free")
    ok = totals["xip_free"] >= XIP_MIN and totals["pool_margin"] >= POOL_MIN and totals.get("xdata_free", 0) >= 0
    out.append(f"engine-memory: {'PASS' if ok else 'FAIL'} {model}")
    return "\n".join(out), ok


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("elf")
    ap.add_argument("--model", default="unknown")
    ap.add_argument("--nm", default=os.environ.get("X0X_NM", str(Path(os.environ.get(
        "JIELI_TOOLCHAIN", Path.home() / ".jieli/toolchain")) / "pi32v2/bin/nm")))
    ap.add_argument("--bin", help="x0x.bin, for the exact image size (default: next to the ELF)")
    a = ap.parse_args(argv)
    elf = Path(a.elf)
    if not elf.is_file():
        print(f"engine-memory: FAIL {a.model} (missing {elf}; NOT_RUN = FAIL)")
        return 1
    binp = Path(a.bin) if a.bin else elf.with_name("x0x.bin")
    image = binp.stat().st_size if binp.is_file() else None
    text = subprocess.run([a.nm, "-S", str(elf)], check=True, capture_output=True, text=True).stdout
    syms, marks = parse_nm(text)
    try:
        table, totals = report(syms, marks, image)
    except ValueError as e:
        print(f"engine-memory: FAIL {a.model} ({e})")
        return 1
    s, ok = render(a.model, table, totals)
    print(s)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
