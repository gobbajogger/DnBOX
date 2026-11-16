#!/usr/bin/env python3
"""Synthetic regression inputs: no licensed factory bytes required."""
import copy
import gzip
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

path = Path(__file__).resolve().parents[2] / 'tools/audit-sj6-factory.py'
spec = importlib.util.spec_from_file_location('audit_sj6', path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class AuditTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.schema = {'parameters': [{'identifier': 'known'}]}
        self.bank = {'name': 'synthetic', 'presets': [{'parameterValues': [{'identifier': 'known', 'value': 0.5}]}]}

    def write(self):
        raw = json.dumps(self.bank).encode()
        data = gzip.compress(raw, mtime=0)
        (self.root / 'fixture.gz').write_bytes(data)
        self.manifest = {'banks': [{'file': 'fixture.gz', 'bank_name': 'synthetic', 'presets': 1,
                         'gzip_bytes': len(data), 'gzip_sha256': module.digest(data),
                         'original_bytes': len(raw), 'original_sha256': module.digest(raw)}],
                         'total_banks': 1, 'total_presets': 1}

    def run_audit(self):
        return module.audit(self.root, self.manifest, self.schema)

    def test_known_is_not_dsp_admission(self):
        self.write()
        self.assertEqual(self.run_audit()['complete_identity_coverage'], 'PASS')
        self.assertEqual(self.run_audit()['dsp_mapping_status'], 'NOT_ADMITTED')

    def test_unknown_retained_and_fails_coverage(self):
        self.bank['presets'][0]['parameterValues'].append({'identifier': 'legacy', 'value': 0})
        self.write()
        result = self.run_audit()
        self.assertEqual(result['unknown_identifiers'], {'legacy': 1})
        self.assertEqual(result['complete_identity_coverage'], 'FAIL')
        self.assertEqual(result['parameter_entries'], 2)

    def test_duplicate_rejected(self):
        self.bank['presets'][0]['parameterValues'] *= 2
        self.write()
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            self.run_audit()

    def test_invalid_values_rejected(self):
        for value in [float('nan'), float('inf'), True, '0']:
            with self.subTest(value=value):
                self.bank['presets'][0]['parameterValues'][0]['value'] = value
                self.write()
                with self.assertRaisesRegex(ValueError, 'non-finite'):
                    self.run_audit()

    def test_corrupt_original_and_compressed_rejected(self):
        self.write()
        for field in ['original_sha256', 'gzip_sha256']:
            with self.subTest(field=field):
                old = self.manifest['banks'][0][field]
                self.manifest['banks'][0][field] = '0' * 64
                with self.assertRaisesRegex(ValueError, 'mismatch'):
                    self.run_audit()
                self.manifest['banks'][0][field] = old

    def test_missing_bank_rejected(self):
        self.write()
        self.manifest['total_banks'] = 2
        with self.assertRaisesRegex(ValueError, 'totals'):
            self.run_audit()

    def test_duplicate_or_escape_file_rejected(self):
        self.write()
        self.manifest['banks'].append(copy.deepcopy(self.manifest['banks'][0]))
        with self.assertRaisesRegex(ValueError, 'path'):
            self.run_audit()
        self.manifest['banks'] = self.manifest['banks'][:1]
        self.manifest['banks'][0]['file'] = '../fixture.gz'
        with self.assertRaisesRegex(ValueError, 'path'):
            self.run_audit()


if __name__ == '__main__':
    unittest.main()
