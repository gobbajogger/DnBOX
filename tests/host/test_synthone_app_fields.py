#!/usr/bin/env python3
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('audit', Path(__file__).resolve().parents[2] / 'tools/audit-synthone-app-fields.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class AppFields(unittest.TestCase):
    def make(self, root, presets):
        data = json.dumps(presets).encode()
        (root / 'bank.json').write_bytes(data)
        entry = {'name': 'bank.json', 'size': len(data), 'count': len(presets),
                 'sha256': hashlib.sha256(data).hexdigest(),
                 'sha': hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()}
        (root / 'MANIFEST.json').write_text(json.dumps({'source_repo': 'trusted/test', 'source_pin': 'fixture', 'files': [entry]}))

    def test_source_fields_and_octave_reduction(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.make(root, [{}, {'frequencyA4': 432, 'isHoldMode': 1, 'modWheelRouting': 2,
                                 'tuningMasterSet': [3, 5, 7, 15, 21, 35]},
                                {'tuningMasterSet': [2 * 2 ** (i / 12) for i in reversed(range(12))]}])
            c = m.audit(root)['counts']
            self.assertEqual(c, {'presets': 3, 'non440_A4': 1, 'non12ET_master_sets': 1,
                                 'missing_master_sets': 1, 'nonzero_stored_hold': 1, 'nondefault_modwheel_routing': 1})

    def test_corruption_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); self.make(root, [{}]); (root / 'bank.json').write_text('[{"frequencyA4":432}]')
            with self.assertRaisesRegex(ValueError, 'hash/size mismatch'):
                m.audit(root)

    def test_invalid_values_rejected(self):
        for preset in [{'frequencyA4': math.nan}, {'tuningMasterSet': [0]},
                       {'tuningMasterSet': [math.inf]}, {'isHoldMode': True}]:
            with self.subTest(preset=preset), tempfile.TemporaryDirectory() as d:
                root = Path(d); self.make(root, [preset])
                with self.assertRaises(ValueError): m.audit(root)

    def test_count_mismatch_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); self.make(root, [{}])
            p = root / 'MANIFEST.json'; manifest = json.loads(p.read_text()); manifest['files'][0]['count'] = 2; p.write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'count mismatch'): m.audit(root)

    def test_duplicate_bank_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); self.make(root, [{}])
            p = root / 'MANIFEST.json'; manifest = json.loads(p.read_text()); manifest['files'] *= 2; p.write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'duplicate bank'): m.audit(root)


if __name__ == '__main__': unittest.main()
