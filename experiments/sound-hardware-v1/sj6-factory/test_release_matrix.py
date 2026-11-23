import copy
import json
from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch
from zipfile import ZipFile

import release_matrix as release


class ReleaseMatrixTests(unittest.TestCase):
    def test_pinned_release_selection(self):
        spec_bytes = (release.HERE / 's4-inputs.json').read_bytes()
        spec = json.loads(spec_bytes)
        audit = json.loads((release.HERE / 'original/bundled-bank-audit.json').read_bytes())
        banks = {}
        for row, checked in zip(spec['programs'], audit['programs']):
            key = checked['bundled_bank_uuid']
            bank = banks.setdefault(key, {'uuid': key, 'name': row['bank'], 'presets': []})
            preset = {'name': row['program'], 'uid': row['program_uid'], 'position': 0,
                      'parameterValues': [{'identifier': 'testWidth', 'value': -0.0}], 'seqTempo': 99}
            bank['presets'].append(preset)
            checked['parameter_count'] = len(preset['parameterValues'])
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            ipa = root / 'synthetic.ipa'
            with ZipFile(ipa, 'w') as archive:
                archive.writestr(release.EXECUTABLE, b'test executable')
                for key, bank in banks.items():
                    archive.writestr(release.PREFIX + key + '.superj6.bank.json', json.dumps(bank))
            with self.assertRaisesRegex(ValueError, 'original build-9'):
                release.prepare_release(ipa)
            audit['ipa_sha256'] = sha256(ipa.read_bytes()).hexdigest()
            audit['au_sha256'] = sha256(b'test executable').hexdigest()
            (root / 'original').mkdir()
            (root / 's4-inputs.json').write_bytes(spec_bytes)
            (root / 'original/bundled-bank-audit.json').write_bytes(release.json_bytes(audit))
            with patch.object(release, 'IPA', audit['ipa_sha256']), patch.object(release, 'HERE', root):
                with self.assertRaisesRegex(ValueError, 'AU executable'):
                    release.prepare_release(ipa)
                with patch.object(release, 'AU', audit['au_sha256']):
                    raw, receipt, files = release.prepare_release(ipa)
                    self.assertEqual((raw, receipt, files), release.prepare_release(ipa))
                    self.assertEqual(sha256(raw).hexdigest(), receipt['spec_sha256'])
                    matrix = json.loads(raw)
                    for name in ('capture_frames', 'sample_rate', 'band_hz', 'gain', 'events',
                                 'block_frames', 'transport', 'reset', 'level_limit_db', 'spectral_limit_db'):
                        self.assertEqual(matrix[name], spec[name])
                    self.assertEqual(len(files), 14)
                    for row, exported, checked in zip(matrix['programs'], spec['programs'], receipt['programs']):
                        selected = files[row['program_uid']]
                        self.assertEqual(sha256(selected).hexdigest(), checked['file_sha256'])
                        self.assertEqual(row['program_uid'], exported['program_uid'])
                        self.assertNotEqual(row['bank_uuid'], exported['bank_uuid'])
                        self.assertEqual(row['position'], 0)
                        preset = json.loads(selected)['preset']
                        self.assertEqual(sha256(release.canonical_bytes(preset)).hexdigest(), row['record_sha256'])
                        self.assertIn(b'-0.0', selected)
                    bad_banks = copy.deepcopy(banks)
                    programs = next(iter(bad_banks.values()))['presets']
                    programs.append(programs[0])
                    with self.assertRaisesRegex(ValueError, 'ambiguous'):
                        release.select_bundled(bad_banks, spec_bytes, audit)
                    with self.assertRaisesRegex(ValueError, 'schedule'):
                        release.select_bundled(banks, spec_bytes + b' ', audit)
