import copy
import importlib.util
from pathlib import Path
import unittest
import tempfile
import json
import gzip
import zipfile

p = Path(__file__).resolve().parents[1] / 'tools/audit-sj6-bundled-legacy.py'
spec = importlib.util.spec_from_file_location('bundled', p)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def banks():
    return [{'name': 'A', 'presets': [{'uid': 'p', 'version': '1.5',
        'parameterValues': [{'identifier': 'neonlimiterAmount', 'value': 0.5}]}]}]


class Comparison(unittest.TestCase):
    def test_equal_does_not_prove_migration(self):
        r = m.compare(banks(), banks())
        self.assertEqual(r['matching_legacy_records'], 1)
        self.assertEqual(r['migration_status'], 'NOT_PROVEN')
        self.assertEqual(r['loader_execution'], 'NOT_RUN')

    def test_different_key_never_aliased(self):
        b = banks(); b[0]['presets'][0]['parameterValues'][0]['identifier'] = 'neoncompressorAmount'
        self.assertEqual(m.compare(banks(), b)['status'], 'FAIL')

    def test_value_version_and_bank_changes_detected(self):
        for field in ('value', 'version'):
            b = banks()
            if field == 'value': b[0]['presets'][0]['parameterValues'][0]['value'] = 0.51
            elif field == 'version': b[0]['presets'][0]['version'] = '1.6'
            else: b[0]['name'] = 'B'
            self.assertEqual(m.compare(banks(), b)['status'], 'FAIL')

    def test_identity_set_mismatch(self):
        b = banks(); b[0]['presets'][0]['uid'] = 'q'
        with self.assertRaisesRegex(ValueError, 'identity sets'): m.compare(banks(), b)

    def test_duplicate_preset_and_bank(self):
        b = banks(); b[0]['presets'].append(copy.deepcopy(b[0]['presets'][0]))
        with self.assertRaisesRegex(ValueError, 'preset identity'): m.records(b)
        with self.assertRaisesRegex(ValueError, 'bank identity'): m.records(banks() + banks())

    def test_duplicate_parameter(self):
        b = banks(); b[0]['presets'][0]['parameterValues'] *= 2
        with self.assertRaisesRegex(ValueError, 'parameter identity'): m.records(b)

    def test_absent_or_coexisting_amount(self):
        for values in ([], [{'identifier': 'neonlimiterAmount', 'value': 0}, {'identifier': 'neontubeRageAmount', 'value': 0}]):
            b = banks(); b[0]['presets'][0]['parameterValues'] = values
            with self.assertRaisesRegex(ValueError, 'exactly one'): m.records(b)

    def test_nonfinite_bool_rejected(self):
        for value in (float('inf'), float('nan'), True):
            b = banks(); b[0]['presets'][0]['parameterValues'][0]['value'] = value
            with self.assertRaisesRegex(ValueError, 'parameter value'): m.records(b)


class InputBinding(unittest.TestCase):
    def fixture(self, root):
        raw = json.dumps(banks()[0]).encode()
        packed = gzip.compress(raw, mtime=0)
        (root / 'A.gz').write_bytes(packed)
        manifest = {'banks': [{'file': 'A.gz', 'bank_name': 'A', 'presets': 1,
            'gzip_bytes': len(packed), 'gzip_sha256': m.sha(packed),
            'original_bytes': len(raw), 'original_sha256': m.sha(raw)}],
            'total_banks': 1, 'total_presets': 1}
        (root / 'manifest.json').write_text(json.dumps(manifest))
        archive = root / 'archive.zip'
        with zipfile.ZipFile(archive, 'w') as z:
            z.writestr('Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/PresetBanks/A.superj6.bank.json', raw)
        return root / 'manifest.json', archive, m.sha(archive.read_bytes())

    def test_complete_binding(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); mf, archive, digest = self.fixture(root)
            r = m.run(root, mf, archive, digest)
            self.assertEqual(r['matching_legacy_records'], 1)
            self.assertEqual(r['manifest_sha256'], m.sha(mf.read_bytes()))
            self.assertEqual(r['bundled_inputs'][0]['sha256'], m.sha(json.dumps(banks()[0]).encode()))

    def test_archive_hash_before_parse(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); mf, archive, digest = self.fixture(root)
            archive.write_bytes(b'changed and not a zip')
            with self.assertRaisesRegex(ValueError, 'archive SHA256'): m.run(root, mf, archive, digest)

    def test_bank_hash_binding(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); mf, archive, digest = self.fixture(root)
            (root / 'A.gz').write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'compressed input'): m.run(root, mf, archive, digest)

    def test_extra_empty_bundled_bank_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); mf, archive, digest = self.fixture(root)
            with zipfile.ZipFile(archive, 'a') as z:
                z.writestr('Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/PresetBanks/Extra.superj6.bank.json', json.dumps({'name': 'Extra', 'presets': []}))
            with self.assertRaisesRegex(ValueError, 'bank identity sets'):
                m.run(root, mf, archive, m.sha(archive.read_bytes()))

    def test_bank_count_binding(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); mf, archive, digest = self.fixture(root)
            doc = json.loads(mf.read_bytes()); doc['total_presets'] = 2
            mf.write_text(json.dumps(doc))
            with self.assertRaisesRegex(ValueError, 'manifest totals'): m.run(root, mf, archive, digest)


if __name__ == '__main__': unittest.main()
