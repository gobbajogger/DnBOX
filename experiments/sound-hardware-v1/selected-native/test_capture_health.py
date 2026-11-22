"""Replay the populated timing capture guards using field bytes from native RAM."""
import json
from pathlib import Path
import unittest

from heavy import capture_health, fault_snapshot


class CaptureHealthTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path = Path(__file__).with_name("capture-health-receipts.json")
        cls.cases = json.loads(path.read_text())["cases"]

    @staticmethod
    def snapshot(phase):
        # Keep the original field bytes and map only their addresses into the
        # small fixture, so the same parser reads all 17 words and the BSS flag.
        ram = bytes.fromhex(phase["record_hex"] + phase["handler_hex"])
        symbols = {"fm1_crash": 0x01c00000, "fm1_fault_c.in_fault": 0x01c00044}
        return fault_snapshot(ram, symbols)

    def test_native_captures(self):
        for row in self.cases:
            with self.subTest(case=row["case"]):
                boot, shot = row["boot"], row["shot"]
                status = capture_health(boot, shot, self.snapshot(boot), self.snapshot(shot))
                self.assertEqual(status, row["measured_health"])
                self.assertEqual(status["passed"], row["expected_complete"])

    def test_retained_fault_and_truncated_audio(self):
        passing = next(row for row in self.cases if row["expected_complete"])
        fault = next(row for row in self.cases if not row["expected_complete"])
        boot, shot = passing["boot"], passing["shot"]
        retained = {**self.snapshot(fault["shot"]), "handler_active": False}
        status = capture_health(boot, shot, retained, retained)
        self.assertTrue(status["passed"])
        self.assertTrue(status["retained_fault"])
        self.assertFalse(capture_health(boot, fault["shot"], retained, retained)["duration_ok"])
        active = {**retained, "handler_active": True}
        self.assertFalse(capture_health(boot, shot, retained, active)["fault_ok"])
        changed = {**retained, "record": {**retained["record"], "count": retained["record"]["count"] + 1}}
        self.assertFalse(capture_health(boot, shot, retained, changed)["fault_ok"])


if __name__ == "__main__":
    unittest.main()
