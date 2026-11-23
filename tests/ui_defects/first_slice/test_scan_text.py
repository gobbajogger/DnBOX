# SPDX-License-Identifier: GPL-3.0-only
"""L14 text scan: questions and instructions on ordinary screens are findings."""
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'ui_geometry'))
from scan_text import is_instruction, scan  # noqa: E402


def log(screens):
    lines = []
    for screen, captions in screens.items():
        for caption in captions:
            lines.append(f"UI_TEXT\t{screen}\t0\t0\t{caption}\n")
        lines.append("AUDIT " + json.dumps({"kind": "screen", "id": screen, "texts": len(captions), "findings": 0}) + "\n")
    lines.append("AUDIT " + json.dumps({"kind": "complete", "screens": len(screens)}) + "\n")
    return lines


class ScanText(unittest.TestCase):
    def test_instructions_and_questions_flagged(self):
        for caption in ("RESET PROJECT?", "EMPTY   WHITE KEY: ADD BAR", "SEQ + SEL: SLIDER LANE",
                        "TAP: NEXT PAGE", "+ KNOB: CLEAR MOTION", "STEP + K2: CHANCE", "HOLD + KEY: ACCENT"):
            with self.subTest(caption=caption):
                self.assertTrue(is_instruction(caption))

    def test_labels_values_and_headers_pass(self):
        for caption in ("EMPTY", "RESET PROJECT", "SAMPLES KEPT; EDITS LOST", "BD: TRACK", "909: ALL TRACKS",
                        "P1: PATTERN", "K5 DECAY 1.0S", "HOLD", "KEY SOUND", "KEYS + SCREEN", "PAD COLOURS",
                        "STEP LENGTH", "L50", "C", "R50", "+0.1st"):
            with self.subTest(caption=caption):
                self.assertFalse(is_instruction(caption))

    def test_scan_reports_ordinary_screens_only(self):
        result = scan(log({"view-06-page-00": ["SONG", "EMPTY   WHITE KEY: ADD BAR"],
                           "help-05": ["TAP: NEXT PAGE"], "modal-0": ["RESET PROJECT?"]}))
        self.assertEqual(sorted(result["known_instructional_copy"]),
                         ["EMPTY   WHITE KEY: ADD BAR", "RESET PROJECT?"])
        clean = scan(log({"view-06-page-00": ["SONG", "EMPTY"], "help-05": ["TAP: NEXT PAGE"]}))
        self.assertEqual(clean["known_instructional_copy"], {})


if __name__ == "__main__":
    unittest.main()
