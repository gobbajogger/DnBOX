#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""tools/engine_memory_report.py: per-engine attribution and the stock budget verdict.

Synthetic nm tables check the rules; when an Elite or Pro ELF from tools/build.py exists
(build/smk37-elite, build/smk37), the real report runs on it as well."""
import os
import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import engine_memory_report as emr  # noqa: E402


def nm(lines, pool_used=335040):
    base = ["01c20000 B _pool_start", f"{0x01c20000 + pool_used:08x} B _pool_end", "01c1fba4 B _bss_end"]
    return "\n".join(base + lines) + "\n"


class Attribution(unittest.TestCase):
    def test_engines_by_name(self):
        for name, engine in [("s1_dry_voice_render", "synthone"), ("juno6_render", "superj6"),
                             ("dnbx_core_note", "superj6"), ("original_chicago_backend_render", "chicago"),
                             ("ck_arp_step", "chicago"), ("original_miami_voice_render_add", "miami"),
                             ("d9_render", "909"), ("x0x_smp_ride", "909"), ("process.909", "909"),
                             ("d808", "808"), ("brk_side", "break"), ("engine_bass_buf", "bass"),
                             ("dly_buf", "fx"), ("original_runtime_poll", "original-runtime"),
                             ("draw_header", "shared"), ("ui_frame", "shared")]:
            with self.subTest(name=name):
                self.assertEqual(emr.engine_of(name), engine)

    def test_areas_overlaps_and_containers(self):
        syms, marks = emr.parse_nm(nm([
            "02000200 00000100 t d9_render",          # flash, 909
            "02000200 00000100 t d9_render_alias",    # same bytes: counted once
            "02000280 00000100 t ck_arp_step",        # half overlaps the 909 range
            "01c08080 000001bc d .L_MergedGlobals",   # container, not attributed
            "01c08080 00000004 d s1_state",           # ordinary RAM, synthone
            "01c30000 00001000 b dly_buf",            # pool, fx
            "01c7c400 00000100 b ble_r",              # other
        ]))
        table, totals = emr.report(syms, marks, image_bytes=0x86EE0 - 2000)
        self.assertEqual(table["909"]["flash"], 0x100)
        self.assertEqual(table["chicago"]["flash"], 0x80)
        self.assertEqual(table["synthone"]["ram"], 4)
        self.assertEqual(table["fx"]["pool"], 0x1000)
        self.assertEqual(table["shared"]["other"], 0x100)
        self.assertEqual(totals["xip_free"], 2000)
        self.assertEqual(totals["pool_margin"], 0x54000 - 335040 - 8192)
        self.assertEqual(totals["ram_used"], 0x1c1fba4 - 0x1c08000)

    def test_budget_boundaries(self):
        syms, marks = emr.parse_nm(nm([], pool_used=0x54000 - 8192 - 512))
        for xip_free, ok in [(1024, True), (1023, False)]:
            with self.subTest(xip_free=xip_free):
                table, totals = emr.report(syms, marks, image_bytes=0x86EE0 - xip_free)
                text, verdict = emr.render("elite", table, totals)
                self.assertEqual(verdict, ok)
                self.assertIn(f"XIP free: {xip_free} B", text)
                self.assertTrue(text.endswith(("PASS" if ok else "FAIL") + " elite"))
        syms, marks = emr.parse_nm(nm([], pool_used=0x54000 - 8192 - 511))
        table, totals = emr.report(syms, marks, image_bytes=0)
        self.assertFalse(emr.render("pro", table, totals)[1])

    def test_missing_pool_symbols_fail_closed(self):
        syms, marks = emr.parse_nm("02000200 00000100 t d9_render\n")
        with self.assertRaises(ValueError):
            emr.report(syms, marks)

    def test_missing_elf_is_fail(self):
        r = subprocess.run([sys.executable, str(ROOT / "tools/engine_memory_report.py"), "/nonexistent/x0x.elf",
                            "--model", "elite"], capture_output=True, text=True)
        self.assertEqual(r.returncode, 1)
        self.assertIn("engine-memory: FAIL elite", r.stdout)


class RealBuilds(unittest.TestCase):
    def test_linked_models(self):
        nm_tool = Path(os.environ.get("JIELI_TOOLCHAIN", Path.home() / ".jieli/toolchain")) / "pi32v2/bin/nm"
        ran = 0
        for model, d in (("elite", "smk37-elite"), ("pro", "smk37")):
            elf = ROOT / "build" / d / "x0x.elf"
            if not (elf.is_file() and nm_tool.is_file()):
                continue
            ran += 1
            r = subprocess.run([sys.executable, str(ROOT / "tools/engine_memory_report.py"), str(elf),
                                "--model", model, "--nm", str(nm_tool)], capture_output=True, text=True)
            print(r.stdout, end="")
            self.assertIn("engine-memory: ", r.stdout)
            for engine in ("chicago", "miami", "909", "synthone", "superj6", "shared"):
                self.assertIn(f"  {engine} ", r.stdout)
        if not ran:
            print("engine-memory: no linked Elite/Pro ELF in build/ (the gate's build step reports them)")


if __name__ == "__main__":
    unittest.main(verbosity=1)
