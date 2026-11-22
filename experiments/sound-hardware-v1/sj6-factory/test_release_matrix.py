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


class InstalledBundleTests(unittest.TestCase):
    def setUp(self):
        temp = TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.app = self.root / 'SuperJ6.app'
        self.exe = self.app / release.EXECUTABLE.removeprefix(release.APP)
        self.exe.parent.mkdir(parents=True)
        self.exe.write_bytes(b'synthetic AU')
        self.banks = self.app / release.PREFIX.removeprefix(release.APP)
        self.banks.mkdir(parents=True)
        self.bank_file = self.banks / 'Test.superj6.bank.json'
        self.bank = {'name': 'Test', 'uuid': 'release-bank', 'author': 'Original'}
        self.preset = {'name': 'Bass', 'uid': 'program', 'originalUid': 'release-parent',
                       'seqTempo': 99, 'position': 0,
                       'parameterValues': [{'identifier': 'neonvolumeControl', 'value': 0.1},
                                           {'identifier': 'cutoff', 'value': -0.0}]}
        old = copy.deepcopy(self.preset)
        old['originalUid'] = 'export-parent'
        old['parameterValues'][0]['value'] = 0.10000000149011612
        exported_bank = dict(self.bank, uuid='export-bank')
        old_bytes = release.json_bytes({'bank': exported_bank, 'preset': old})
        self.bank['presets'] = [self.preset]
        self.write_bank()
        self.ipa = self.root / 'test.ipa'
        with ZipFile(self.ipa, 'w') as z:
            z.write(self.exe, release.EXECUTABLE)
            z.write(self.bank_file, release.PREFIX + self.bank_file.name)
        ipa_digest = sha256(self.ipa.read_bytes()).hexdigest()
        au_digest = sha256(self.exe.read_bytes()).hexdigest()
        schedule = {'archive_sha256': 'export', 'programs': [
            {'bank': 'Test', 'bank_uuid': 'export-bank', 'program_uid': 'program', 'program': 'Bass',
             'record_sha256': sha256(release.canonical_bytes(old)).hexdigest()}]}
        spec = release.json_bytes(schedule)
        row = {'uid': 'program', 'file_sha256': sha256(old_bytes).hexdigest(),
               'bundled_bank_uuid': 'release-bank', 'supplied_bank_uuid': 'export-bank',
               'parameter_count': 2, 'metadata_differences': ['originalUid'],
               'decimal_differences': ['neonvolumeControl']}
        audit = {'spec_sha256': sha256(spec).hexdigest(), 'ipa_sha256': ipa_digest,
                 'au_sha256': au_digest, 'component_version': 67072, 'programs': [row]}
        changes = {'uid': 'program', 'metadata_changes': {
            'originalUid': {'release': 'release-parent', 'export': 'export-parent'}}}
        (self.root / 'original').mkdir()
        (self.root / 's4-inputs.json').write_bytes(spec)
        (self.root / 'original/bundled-bank-audit.json').write_bytes(release.json_bytes(audit))
        (self.root / 'original/all-programs.json').write_bytes(release.json_bytes({
            'ipa_sha256': ipa_digest, 'au_sha256': au_digest, 'export_zip_sha256': 'export',
            'program_differences': [changes]}))
        for name, value in [('HERE', self.root), ('AU', au_digest), ('IPA', ipa_digest)]:
            override = patch.object(release, name, value)
            override.start()
            self.addCleanup(override.stop)

    def write_bank(self):
        self.bank_file.write_bytes(release.json_bytes(self.bank))

    def test_installed_records_match_ipa_without_claiming_archive_verification(self):
        expected, ipa_audit, records = release.prepare_release(self.ipa)
        actual, installed_audit, installed = release.prepare_release(self.app)
        self.assertEqual((expected, records), (actual, installed))
        self.assertEqual(installed_audit['source_kind'], 'installed_app')
        self.assertNotIn('ipa_sha256', installed_audit)
        self.assertEqual(installed_audit['reference_ipa_sha256'], ipa_audit['ipa_sha256'])
        self.assertEqual(installed_audit['programs'], ipa_audit['programs'])
        self.assertEqual(json.loads(installed['program'])['preset'], self.preset)

    def test_macos_layout_and_ios_wrapper(self):
        expected = release.prepare_release(self.app)
        self.assertEqual(expected, release.prepare_release(self.exe.parent))
        wrapper = self.root / 'Wrapped.app' / 'Wrapper'
        wrapper.mkdir(parents=True)
        moved = self.app.rename(wrapper / self.app.name)
        self.assertEqual(expected, release.prepare_release(wrapper.parent))
        content = self.app / 'Contents'
        new_exe = content / 'PlugIns/SuperJ6 AU.appex/Contents/MacOS/SuperJ6 AU'
        new_exe.parent.mkdir(parents=True)
        (moved / self.exe.relative_to(self.app)).rename(new_exe)
        new_banks = content / 'Frameworks/SuperJ6_Assets.framework/Resources/PresetBanks'
        new_banks.parent.mkdir(parents=True)
        (moved / self.banks.relative_to(self.app)).rename(new_banks)
        self.assertEqual(expected, release.prepare_release(self.app))

    def test_rejects_mutated_banks_even_with_the_verified_executable(self):
        original = copy.deepcopy(self.bank)
        for case in ('bank', 'volume', 'export_spelling', 'cutoff', 'order', 'metadata', 'missing'):
            self.bank = copy.deepcopy(original)
            p = self.bank['presets'][0]
            if case == 'bank':
                self.bank['author'] = 'Changed'
            elif case == 'volume':
                p['parameterValues'][0]['value'] = 0.10000002
            elif case == 'export_spelling':
                p['parameterValues'][0]['value'] = 0.10000000149011612
            elif case == 'cutoff':
                p['parameterValues'][1]['value'] = 0.0
            elif case == 'order':
                p['parameterValues'].reverse()
            elif case == 'metadata':
                p['originalUid'] = 'export-parent'
            else:
                self.bank['uuid'] = 'wrong'
            self.write_bank()
            with self.subTest(case=case), self.assertRaises(ValueError):
                release.prepare_release(self.app)

    def test_rejects_changed_executable_and_duplicate_bank(self):
        self.exe.write_bytes(b'different AU')
        with self.assertRaisesRegex(ValueError, 'AU executable'):
            release.prepare_release(self.app)
        self.exe.write_bytes(b'synthetic AU')
        (self.banks / 'dup.superj6.bank.json').write_bytes(self.bank_file.read_bytes())
        with self.assertRaisesRegex(ValueError, 'ambiguous'):
            release.prepare_release(self.app)
