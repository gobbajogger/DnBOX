import hashlib
import unittest
from inspect_flash import inspect, FLASH_BYTES, REGIONS

class ReadOnlyRegions(unittest.TestCase):
    def setUp(self):
        self.raw = bytearray(b"\xff" * FLASH_BYTES)

    def record(self, name, tag, at, body):
        lo, hi = REGIONS[name]
        self.raw[lo:lo+len(tag)] = tag
        self.raw[lo+at:lo+at+len(body)] = body

    def test_blank_count_is_only_snapshot_evidence(self):
        got = inspect(self.raw)
        self.assertFalse(got["write_authorized"])
        self.assertEqual(got["regions"]["factory_data"]["raw_room"], 6133)
        self.assertEqual(got["regions"]["sample_slot_3"]["raw_room"], 961)
        self.assertTrue(all(r["blank_in_snapshot"] for r in got["regions"].values()))

    def test_occupied_known_data_not_available(self):
        self.record("factory_data", b"S1PK", 61207, b"\0")
        self.record("sample_slot_3", b"FSMP", 4095, b"\0")
        before = hashlib.sha256(self.raw).hexdigest()
        got = inspect(self.raw)
        self.assertEqual(got["snapshot_sha256"], before)
        self.assertEqual(hashlib.sha256(self.raw).hexdigest(), before)
        self.assertFalse(got["write_authorized"])
        self.assertEqual(got["regions"]["factory_data"]["non_erased_extent"], 61208)
        self.assertEqual(got["regions"]["sample_slot_3"]["non_erased_extent"], 4096)

    def test_erased_magic_does_not_prove_empty(self):
        self.record("sample_slot_3", b"", 32767, b"\x7f")
        got = inspect(self.raw)["regions"]["sample_slot_3"]
        self.assertEqual(got["status"], "occupied; do not replace")
        self.assertEqual(got["non_erased_extent"], 32768)
        self.assertIn("unknown", got["content"])

    def test_short_or_long_snapshot_rejected(self):
        for size in (0, FLASH_BYTES - 1, FLASH_BYTES + 1):
            with self.assertRaises(ValueError):
                inspect(bytes(size))

if __name__ == "__main__":
    unittest.main()
