#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Repeatable stock BREAK fit proof against pinned native builds, not a new target build.

Run: python3 -m unittest discover -s tests -p starting_break_stock_fit_checkpoint_test.py -v
The metadata-only case needs no SDK; the canonical-bank case requires the repo inputs.
"""
import hashlib
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SNAPSHOT = ROOT / "evidence/integ-resume/starting-breaks-checkpoint.json"
HISTORICAL_BUILDS = ROOT / "evidence/integ-starting-breaks/native-builds.json"


class StockBreakFit(unittest.TestCase):
    def test_pinned_native_build_capacity(self):
        snapshot = json.loads(SNAPSHOT.read_text())
        builds = json.loads(HISTORICAL_BUILDS.read_text())
        self.assertEqual(builds["combined"], snapshot["native_source_commit"])
        self.assertEqual(len(snapshot["models"]), 2)
        for name in ("elite", "pro"):
            with self.subTest(model=name):
                model = builds["models"][name]
                recorded = snapshot["models"][name]
                app = model["files"]["x0x.bin"]
                self.assertEqual(app["sha256"], recorded["application_sha256"])
                self.assertEqual(app["bytes"], recorded["application_bytes"])
                aligned = (app["bytes"] + 15) & ~15
                available = max(0, snapshot["code_max"] - snapshot["xip_reserve"] - aligned)
                self.assertEqual(available, model["budget_1k_reserved"])
                self.assertEqual(available, recorded["available_bytes"])
                self.assertGreater(snapshot["bank_bytes"], available)
                self.assertEqual(snapshot["bank_bytes"] - available, recorded["shortfall_bytes"])
                self.assertGreaterEqual(model["free_xip"], 1024)

    def test_canonical_bank_and_native_rejection(self):
        # Uses the *actual* bank, encoder/parser and stock flash limit, not a fake pack.
        sys.path.insert(0, str(ROOT / "tools"))
        import break_pack
        import fm1pkg_make

        snapshot = json.loads(SNAPSHOT.read_text())
        builds = json.loads(HISTORICAL_BUILDS.read_text())
        self.assertEqual(fm1pkg_make.BOARDS["smk37"][3], snapshot["code_max"])
        self.assertEqual(break_pack.SMK_XIP_RESERVE, snapshot["xip_reserve"])
        bank = break_pack.STARTING_PACK.read_bytes()
        self.assertEqual(break_pack.STARTING_SHA256, snapshot["bank_sha256_pinned_by_break_pack_py"])
        self.assertEqual(hashlib.sha256(bank).hexdigest(), break_pack.STARTING_SHA256)
        self.assertEqual(len(bank), snapshot["bank_bytes"])
        parsed = break_pack.parse_pack(bank)
        self.assertIsNotNone(parsed)
        self.assertEqual(parsed[0], 22_050)
        self.assertEqual([s["name"] for s in parsed[1]],
                         ["BR1 1", "BR1 2", "BR1 3", "BR2 1", "BR2 2"])
        for name in ("elite", "pro"):
            with self.subTest(model=name):
                bytes_used = builds["models"][name]["files"]["x0x.bin"]["bytes"]
                available = snapshot["models"][name]["available_bytes"]
                with self.assertRaisesRegex(ValueError,
                        rf"loop pack needs {len(bank)} bytes, {available} free"):
                    break_pack.embed_starting_breaks(bytes(bytes_used), snapshot["code_max"])
        # Synthetic exact-capacity control is NOT a production build or hardware proof.
        max_aligned_start = (snapshot["code_max"] - snapshot["xip_reserve"] - len(bank)) & ~15
        accepted = break_pack.embed_starting_breaks(bytes(max_aligned_start), snapshot["code_max"])
        self.assertEqual(accepted[max_aligned_start:], bank)
        with self.assertRaises(ValueError):
            break_pack.embed_starting_breaks(bytes(max_aligned_start + 1), snapshot["code_max"])


if __name__ == "__main__":
    unittest.main()
