#!/usr/bin/env python3
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('reference', REPO / 'tools/run-synthone-tuning-reference.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
FIXTURE = REPO / 'tests/fixtures/synthone-tuning'


class ReferenceClosure(unittest.TestCase):
    def test_original_math_unchanged_except_objc_annotation(self):
        files = m.verify_inputs(FIXTURE)
        generated, changes = m.swift_source(files, b'// harness')
        for name, change in changes.items():
            original = files[name]
            transformed = original.replace(b'@objc ', b'')
            self.assertIn(transformed, generated)
            self.assertEqual(m.sha(transformed), change['compiled_text_sha256'])
        self.assertIn(b'return Swift.min(Swift.max(value, lowerBound), upperBound)', generated)
        self.assertIn(b'return !a', generated)
        self.assertIn(b'public typealias MIDINoteNumber = UInt8', generated)

    def test_changed_original_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d) / 'fixture'; shutil.copytree(FIXTURE, root)
            name = json.loads((root / 'SOURCE-MANIFEST.json').read_text())['files'][0]['path']
            (root / 'upstream' / name).write_text('// changed')
            with self.assertRaisesRegex(ValueError, 'original source mismatch'): m.verify_inputs(root)

    def test_traversal_rejected(self):
        for name in ['/tmp/escape', '../escape', 'a/../../escape']:
            with self.subTest(name=name), self.assertRaises(ValueError): m.checked_relative(name)

    def test_ambiguous_original_fragment_rejected(self):
        with self.assertRaisesRegex(ValueError, 'not unique'):
            m.block('extension ClosedRange {} extension ClosedRange {}', 'extension ClosedRange {')

    def test_staged_change_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / 'stage'; receipt = m.stage(REPO, out)
            m.verify_stage(out, receipt)
            (out / 'main.swift').write_text('// changed')
            with self.assertRaisesRegex(ValueError, 'staged input changed'): m.verify_stage(out, receipt)

    def test_case_inputs_bind_original_bank_bytes(self):
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / 'stage'; receipt = m.stage(REPO, out)
            cases = json.loads((out / 'cases.json').read_text())
            self.assertEqual(len(cases), 694)
            manifest = json.loads((out / 'banks/MANIFEST.json').read_text())
            expected = []
            for e in manifest['files']:
                data = (out / 'banks' / e['name']).read_bytes()
                self.assertEqual(m.sha(data), e['sha256'])
                for i, preset in enumerate(json.loads(data)):
                    expected.append({'id': e['name'] + ':' + str(i), 'master': preset.get('tuningMasterSet')})
            self.assertEqual(cases, expected)
            (out / 'banks/BankA.json').write_text('[]')
            with self.assertRaisesRegex(ValueError, 'staged input changed'): m.verify_stage(out, receipt)


if __name__ == '__main__': unittest.main()
