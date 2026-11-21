#!/usr/bin/env python3
"""Regression tests for real provenance, coverage and overwrite defects."""
from pathlib import Path
import copy
import importlib.util
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("dry_check", HERE / "check.py")
check = importlib.util.module_from_spec(spec)
spec.loader.exec_module(check)


class ComparisonIntegrity(unittest.TestCase):
    def setUp(self):
        self.pcm = bytes(check.MONO_BYTES * 2)
        self.report = {"gain": 1, "nonfinite": 0, "pcm_sha256": check.digest(self.pcm),
                       "rows": [{"note": n, "rms_db": 0.0, "spectral_db": 0.0}
                                for n in check.NOTES]}

    def test_missing_or_duplicate_note_cannot_pass(self):
        self.assertEqual(check.validate_report(self.report, self.pcm), [True] * 3)
        for notes in ([36, 60], [36, 60, 60], [60, 36, 84], []):
            report = copy.deepcopy(self.report)
            report["rows"] = [{"note": n, "rms_db": 0.0, "spectral_db": 0.0} for n in notes]
            with self.assertRaisesRegex(RuntimeError, "note"):
                check.validate_report(report, self.pcm)

    def test_changed_gain_pcm_or_nonfinite_count_rejected(self):
        for key, value, message in (("gain", 2, "gain"), ("pcm_sha256", "0" * 64, "hash"),
                                    ("nonfinite", 1, "count")):
            report = copy.deepcopy(self.report)
            report[key] = value
            with self.assertRaisesRegex(RuntimeError, message):
                check.validate_report(report, self.pcm)
        with self.assertRaisesRegex(RuntimeError, "length"):
            check.validate_report(self.report, b"")

    def test_one_failed_note_fails_file_without_relaxing_limits(self):
        for key, value in (("rms_db", 1.000001), ("spectral_db", 3.000001)):
            report = copy.deepcopy(self.report)
            report["rows"][2][key] = value
            self.assertEqual(check.validate_report(report, self.pcm), [True, True, False])
        self.report["rows"][0]["rms_db"] = float("nan")
        with self.assertRaisesRegex(RuntimeError, "metric"):
            check.validate_report(self.report, self.pcm)

    def test_existing_or_symlink_output_preserves_sentinel(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            existing = root / "existing"
            existing.mkdir()
            sentinel = existing / "sentinel"
            sentinel.write_text("keep")
            link = root / "link"
            link.symlink_to(existing, target_is_directory=True)
            dangling = root / "dangling"
            dangling.symlink_to(root / "absent", target_is_directory=True)
            for path in (existing, link, dangling, link / "new-child"):
                with self.assertRaisesRegex(RuntimeError, "exists|Symlink"):
                    check.fresh_directory(path)
            self.assertEqual(sentinel.read_text(), "keep")
            self.assertFalse((existing / "new-child").exists())


if __name__ == "__main__":
    unittest.main()
