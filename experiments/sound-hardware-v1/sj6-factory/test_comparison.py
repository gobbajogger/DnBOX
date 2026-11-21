import hashlib
import json
from pathlib import Path
import tempfile
import unittest

import numpy as np

from compare import SPEC, capture, channel_score, compare


class ComparisonTests(unittest.TestCase):
    def test_identity_gain_and_frequency(self):
        rng = np.random.default_rng(734)
        source = rng.normal(0, 0.1, 44100).astype("<f4")
        level, spectral = channel_score(source, source, 44100, [20, 16000])
        self.assertEqual((level, spectral), (0., 0.))
        level, _ = channel_score(source, source*.5, 44100, [20, 16000])
        self.assertAlmostEqual(level, -6.0206, places=4)
        quiet = np.zeros_like(source)
        self.assertEqual(channel_score(source, quiet, 44100, [20, 16000]), (None, None))
        # Preserve level while removing high frequencies.
        filtered = np.convolve(source, np.ones(9)/9, mode="same")
        filtered *= np.linalg.norm(source)/np.linalg.norm(filtered)
        level, spectral = channel_score(source, filtered, 44100, [20, 16000])
        self.assertLess(abs(level), 0.01)
        self.assertGreater(spectral, 3.)

    def test_missing_captures_and_wrong_record(self):
        spec = json.loads(SPEC.read_bytes())
        spec_hash = hashlib.sha256(SPEC.read_bytes()).hexdigest()
        row = spec["programs"][0]
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp)
            self.assertEqual(compare(target, target)["scored"], 0)
            rec = {"arm": "original", "bank_uuid": row["bank_uuid"],
                   "program_uid": row["program_uid"], "record_sha256": "wrong"}
            (target / (row["program_uid"] + ".json")).write_text(json.dumps(rec))
            with self.assertRaisesRegex(ValueError, "record_sha256"):
                capture(target, row, spec, spec_hash, "original")


if __name__ == "__main__":
    unittest.main()
