# SPDX-License-Identifier: GPL-3.0-only
"""Synthetic contract checks. No original sample or successful capture is implied."""
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from zipfile import ZipFile

import numpy as np

import factory_batch as batch
from compare import SPEC, control_words, digest
from factory import canonical_bytes


class FactoryBatchTests(unittest.TestCase):
    def setUp(self):
        temp = TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.original = self.root/'original'
        self.original.mkdir()
        self.s4dir = self.root/'s4'
        self.s4dir.mkdir()
        self.uid = 'AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE'
        self.record = {'uid': self.uid, 'seqTempo': 90,
                       'parameterValues': [{'identifier': 'control', 'value': 0.5}]}
        self.bundled = canonical_bytes({'bank': {'uuid': 'bank-1'}, 'preset': self.record}) + b'\n'
        self.row = {'bank': 'test', 'bank_uuid': 'bank-1', 'program_uid': self.uid, 'program': 'test',
                    'position': 0, 'record_sha256': digest(canonical_bytes(self.record))}
        self.spec = {k: json.loads(SPEC.read_bytes())[k] for k in batch.SCHEDULE_KEYS}
        self.spec.update(scope='superj6-factory-640', programs=[self.row])
        self.raw = batch.json_bytes(self.spec)
        self.au = digest(b'test-au')
        self.audit = {'spec_sha256': digest(self.raw), 'ipa_sha256': digest(b'test-IPA'),
                      'au_sha256': self.au, 'component_version': 1,
                      'programs': [{'uid': self.uid, 'file_sha256': digest(self.bundled), 'parameter_count': 1,
                                    'supplied_bank_uuid': 'bank-1', 'bundled_bank_uuid': 'bank-1'}]}
        self.inputs = {self.uid: self.bundled}
        self.batch = self.raw, self.audit, self.inputs
        self.pcm = np.tile(np.array([[0.1, 0.2]], dtype='<f4'), (self.spec['capture_frames'], 1)).tobytes()

    def write_capture(self, directory, raw, pcm=None, **overrides):
        if pcm is None:
            pcm = self.pcm
        active = canonical_bytes(self.record)
        rec = {k: self.spec[k] for k in ('sample_rate', 'channels', 'block_frames', 'reset', 'transport', 'gain')}
        rec.update(arm='original', bank_uuid='bank-1', program_uid=self.uid, record_sha256=self.row['record_sha256'],
                   s4_inputs_sha256=digest(raw), frames=self.spec['capture_frames'], format='stereo-f32le',
                   complete_preset=True, first_event_frame=0, first_render_frame=0,
                   event_count=len(self.spec['events']), scheduled_events=self.spec['events'],
                   au_sha256=self.au, release_au_sha256=self.au, bundled_bank_uuid='bank-1',
                   component_version=1, factory_route='fullState', input_file_sha256=digest(self.bundled),
                   authored_tempo=90, applied_tempo=90, applied_state_sha256=digest(active), pcm_sha256=digest(pcm),
                   applied_readback={'qualified': True, 'authored': 1, 'live_matches': 1, 'missing_active': [],
                                     'active_differences': [], 'missing_live': [], 'live_differences': [],
                                     'unexposed_legacy': [], 'live_words': control_words(self.record['parameterValues'])})
        rec.update(overrides)
        (directory/(self.uid+'.json')).write_bytes(batch.json_bytes(rec))
        (directory/(self.uid+'.applied.json')).write_bytes(active)
        (directory/(self.uid+'.f32le')).write_bytes(pcm)

    def s4_plan(self, **changes):
        spec = dict(self.spec, scope='s4', programs=[self.row] + [
            dict(self.row, program_uid=f'other-{i}') for i in range(13)])
        spec.update(changes)
        raw = batch.json_bytes(spec)
        return raw, dict(self.audit, spec_sha256=digest(raw)), self.inputs

    def test_verified_lossless_archive(self):
        self.write_capture(self.original, self.raw)
        output = self.root/'batch.zip'
        report = batch.pack(self.batch, self.original, output, count=1)
        self.assertEqual(report['archive_bytes'], output.stat().st_size)
        self.assertEqual(batch.verify_published_archive(output, report['archive_sha256']), report)
        with self.assertRaisesRegex(ValueError, 'archive digest'):
            batch.verify_published_archive(output, '0'*64)
        with ZipFile(output) as archive:
            m = json.loads(archive.read('manifest.json'))
            self.assertEqual(len(m['files']), 3)
            self.assertEqual(m['programs'][0]['receipt_scope'], 'factory')
            self.assertEqual(archive.read(self.uid+'.f32le'), self.pcm)
        with self.assertRaises(FileExistsError):
            batch.pack(self.batch, self.original, output, count=1)

    def test_reuse_validates_stimulus_identity_and_never_masks_partial_output(self):
        s4 = self.s4_plan()
        self.write_capture(self.s4dir, s4[0])
        capture, = batch.originals_for_batch(self.batch, self.original, (self.s4dir, s4), count=1)
        self.assertEqual(capture[3:], ('s4', digest(s4[0])))
        with self.assertRaisesRegex(ValueError, 'S4 stimulus'):
            bad_schedule = self.s4_plan(gain=0.5)
            list(batch.originals_for_batch(self.batch, self.original, (self.s4dir, bad_schedule), count=1))
        with self.assertRaisesRegex(ValueError, 'record bytes'):
            list(batch.originals_for_batch(self.batch, self.original, (self.s4dir, (*s4[:2], {self.uid: b'wrong'})), count=1))
        (self.original/(self.uid+'.f32le')).write_bytes(self.pcm)
        with self.assertRaises(FileNotFoundError):
            list(batch.originals_for_batch(self.batch, self.original, (self.s4dir, s4), count=1))

    def test_archived_s4_receipt_keeps_its_matrix_and_scores(self):
        s4 = self.s4_plan()
        self.write_capture(self.s4dir, s4[0])
        packed = self.root/'reuse.zip'
        batch.pack(self.batch, self.original, packed, (self.s4dir, s4), count=1)
        with ZipFile(packed) as z:
            manifest = json.loads(z.read('manifest.json'))
            self.assertEqual(manifest['programs'][0]['receipt_matrix_sha256'], digest(s4[0]))
            for entry in manifest['files']:
                (self.original/entry['path']).write_bytes(z.read(entry['path']))
        dnbox = self.root/'dnbox'
        dnbox.mkdir()
        image = self.root/'test.fwsc'
        image.write_bytes(b'synthetic candidate')
        commit = '1'*40
        self.write_capture(dnbox, self.raw, arm='dnbox', source_commit=commit,
                           firmware_sha256=digest(image.read_bytes()))
        result = batch.score_batch(self.batch, self.original, dnbox, image, commit, 0, 1,
                                   (self.original, s4))
        self.assertEqual(result['passed'], 1)
        self.assertEqual(result['programs'][0]['original_receipt_scope'], 's4')

    def test_factory_matrix_requires_exact_authenticated_factory_shape(self):
        # Reuse identical synthetic control vectors to keep fixture memory small.
        controls = [{'identifier': f'control-{i}', 'value': 0.25} for i in range(142)]
        banks, audit = {}, {'banks': [], 'unpaired_programs': []}
        for j in range(12):
            name = f'bank-{j}'
            banks[name] = {'uuid': name, 'name': name, 'presets': []}
        for i in range(640):
            name = f'bank-{i % 12}'
            banks[name]['presets'].append({'uid': f'AAAAAAAA-BBBB-CCCC-DDDD-{i:012X}', 'name': f'Test {i}',
                                          'position': i, 'parameterValues': controls[:142 if i < 309 else 141]})
        for b in banks.values():
            audit['banks'].append({'name': b['name'], 'release_bank_uuid': b['uuid'],
                                   'release_programs': len(b['presets'])})
        banks['User Bank'] = {'presets': [{'uid': 'user-only'}]}
        audit['unpaired_programs'].append({'present_only_in': 'release', 'bank': 'User Bank', 'uid': 'user-only'})
        raw, receipt, inputs = batch.factory_matrix(banks, self.spec, audit, 1)
        self.assertEqual(len(inputs), 640)
        self.assertEqual(len(receipt['programs']), 640)
        self.assertEqual(sum(r['parameter_count'] for r in receipt['programs']), 90549)
        self.assertEqual(digest(raw), receipt['spec_sha256'])
        self.assertNotIn('user-only', inputs)
        banks['bank-0']['presets'][1]['uid'] = banks['bank-0']['presets'][0]['uid']
        with self.assertRaisesRegex(ValueError, 'duplicate factory UID'):
            batch.factory_matrix(banks, self.spec, audit, 1)

    def test_invalid_pcm_never_leaves_an_archive(self):
        self.write_capture(self.original, self.raw, pcm=self.pcm[:-4])
        path = self.root/'invalid.zip'
        with self.assertRaisesRegex(ValueError, 'PCM length'):
            batch.pack(self.batch, self.original, path, count=1)
        self.assertFalse(path.exists())

    def test_scores_independently_pinned_candidate(self):
        self.write_capture(self.original, self.raw)
        dnbox = self.root/'dnbox'
        dnbox.mkdir()
        image = self.root/'test.fwsc'
        image.write_bytes(b'synthetic candidate')
        image_sha = digest(image.read_bytes())
        commit = '1'*40
        self.write_capture(dnbox, self.raw, arm='dnbox', source_commit=commit, firmware_sha256=image_sha)
        result = batch.score_batch(self.batch, self.original, dnbox, image, commit, 0, 1)
        self.assertEqual(result['passed'], 1)
        self.assertEqual(result['programs'][0]['channels'], [(0., 0.), (0., 0.)])
        with self.assertRaisesRegex(ValueError, 'source_commit'):
            batch.score_batch(self.batch, self.original, dnbox, image, '2'*40, 0, 1)


if __name__ == '__main__':
    unittest.main()
