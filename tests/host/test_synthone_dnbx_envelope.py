#!/usr/bin/env python3
"""Check the independent DnBOX S1 envelope against immutable numeric captures.

These tests catch inherited SJ6 timing, live-time relatching, sustain snaps,
retrigger resets, lost tiny release tails, and sharing between envelope owners.
They qualify this component and the supplied captures, not a whole instrument.
"""
from __future__ import annotations

import ctypes as C
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
STATIC = ROOT / 'experiments/sound-hardware-v1/mapping/synthone_followup/implementer_brief'
LIVE = ROOT / 'tests/fixtures/synthone-envelope-live'
HASHES = {
    'static': '9b164e9b7ebf3fe95665ceaa64afb39d51442e4167305c5d7bc55e115995ade4',
    'live': '2cafbc8eb4b7aba8e22e0820ce8116d9dc7a57e6f8c41ed162c44ed98c474cfe',
    'static_metadata': '108f75f3965c95e1d3e2259e9d70f076bea4ca18843ec0af29e1477c0dd6d163',
    'live_metadata': '03bbf96fd3171db6af5854241b7ec3272ecff3f44161dc2caa282070b2882390',
    'live_plan': '336e0313c6b1a4f3fb39a232f6ca74f7a53c2103cdedb225518070740dd3f4a3',
}
F32 = struct.Struct('<f')


def f32(x):
    return F32.unpack(F32.pack(x))[0]


def fixtures(static_dir=STATIC, live_dir=LIVE):
    paths = {
        'static': static_dir / 's1_envelope_observations.f32le',
        'live': live_dir / 'reference_observations.f32le',
        'static_metadata': static_dir / 'envelope_observations.json',
        'live_metadata': live_dir / 'observations.json',
        'live_plan': live_dir / 'case_plan.json',
    }
    inputs = {kind: path.read_bytes() for kind, path in paths.items()}
    for kind, data in inputs.items():
        if hashlib.sha256(data).hexdigest() != HASHES[kind]:
            raise AssertionError('Independent reference identity changed: ' + kind)
    meta = json.loads(inputs['static_metadata'])
    live_meta = json.loads(inputs['live_metadata'])
    static, live = inputs['static'], inputs['live']
    cases = []
    for c in meta['cases']:
        start, count = c['byte_offset'], c['frames']
        cases.append(dict(id='static-' + str(c['case']), settings=c['settings_ADSR'],
                          frames=count, off=c['note_off_frame'], event={},
                          reference=static[start:start + count * F32.size]))
    for c in live_meta['cases']:
        start, count = c['byte_offset'], live_meta['frames_per_case']
        cases.append(dict(id='live-' + str(c['id']), settings=live_meta['settings_ADSR'],
                          frames=count, off=live_meta['gate_off_frame'], event=c['stimulus'],
                          reference=live[start:start + count * F32.size]))
    return cases


def difference(expected, actual):
    if len(expected) != len(actual):
        return {'same_length': False, 'expected_bytes': len(expected), 'actual_bytes': len(actual)}
    count = 0
    first = None
    maximum = 0.0
    for i, ((r,), (c,)) in enumerate(zip(F32.iter_unpack(expected),
                                         F32.iter_unpack(actual))):
        start = F32.size * i
        if expected[start:start + F32.size] != actual[start:start + F32.size]:
            count += 1
            if first is None:
                first = i
        if not math.isfinite(c):
            return {'finite': False, 'frame': i}
        maximum = max(maximum, abs(r - c))
    return {'same_length': True, 'finite': True, 'different_words': count,
            'first_different_frame': first, 'max_absolute_error': maximum}


class State(C.Structure):
    _fields_ = [('level', C.c_float), ('retention', C.c_float),
                ('stage', C.c_uint8), ('previous_gate', C.c_uint8),
                ('reserved', C.c_uint8 * 2)]


class Prepared(C.Structure):
    _fields_ = [('attack_retention', C.c_float), ('decay_retention', C.c_float),
                ('sustain', C.c_float), ('release_retention', C.c_float)]


def reference_coefficients(settings):
    # Independent numeric contract only. This deliberately does not call the
    # candidate's target math or derive expected audio from the candidate.
    a, d, s, r = (f32(v) for v in settings)
    return tuple(f32(math.exp(-1.0 / ((44100 * t) * scale))) if t else 0.0
                 for t, scale in [(a, .6), (d, 1)]) + (s,) + \
        (f32(math.exp(-1.0 / (44100 * r))) if r else 0.0,)


class EnvelopeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='dnbox-s1-envelope-')
        cls.addClassCleanup(cls.temp.cleanup)
        binary = Path(cls.temp.name) / 'envelope.so'
        command = [os.environ.get('CC', 'cc'), '-std=c11', '-O2', '-fPIC', '-shared',
                       '-Wall', '-Wextra', '-Werror', '-fno-fast-math', '-ffp-contract=off',
                       '-I' + str(ROOT / 'firmware/src/dsp'),
                       str(ROOT / 'firmware/src/dsp/dnbx_s1_envelope.c'), '-o', str(binary)]
        cp = subprocess.run(command, text=True, capture_output=True)
        if cp.returncode:
            raise AssertionError('Component build failed: ' + cp.stdout + cp.stderr)
        cls.lib = C.CDLL(str(binary))
        cls.lib.dnbx_s1_envelope_reset.argtypes = [C.POINTER(State)]
        cls.lib.dnbx_s1_envelope_reset.restype = None
        cls.lib.dnbx_s1_envelope_configure.argtypes = [C.POINTER(Prepared)] + [C.c_float] * 4
        cls.lib.dnbx_s1_envelope_configure.restype = C.c_int
        cls.lib.dnbx_s1_envelope_sample.argtypes = [C.POINTER(State), C.POINTER(Prepared), C.c_uint]
        cls.lib.dnbx_s1_envelope_sample.restype = C.c_float
        cls.cases = fixtures()
        cls.cases_by_id = {case['id']: case for case in cls.cases}

    def new_state(self):
        state = State()
        self.lib.dnbx_s1_envelope_reset(C.byref(state))
        return state

    def prepared(self, settings):
        p = Prepared()
        self.assertEqual(self.lib.dnbx_s1_envelope_configure(C.byref(p),
                         *reference_coefficients(settings)), 0)
        return p

    def samples(self, case):
        state = self.new_state()
        settings = list(case['settings'])
        p = self.prepared(settings)
        event = case['event']
        for frame in range(case['frames']):
            if frame == event.get('change_frame', -1):
                settings[event['parameter']] = event['value']
                p = self.prepared(settings)
            gate = frame < case['off'] and frame != event.get('gap_frame', -1)
            yield self.lib.dnbx_s1_envelope_sample(C.byref(state), C.byref(p), gate)

    def test_exact_static_and_live_observations(self):
        for case in self.cases:
            with self.subTest(case=case['id']):
                rendered = b''.join(F32.pack(v) for v in self.samples(case))
                if rendered != case['reference']:
                    self.assertEqual(rendered, case['reference'],
                                     msg=json.dumps(difference(case['reference'], rendered)))

    def test_interleaved_owners_reproduce_each_solo_recording(self):
        a, b = self.cases_by_id['live-3'], self.cases_by_id['live-5']
        left = self.samples(a)
        right = self.samples(b)
        out_a, out_b = bytearray(), bytearray()
        for x, y in zip(left, right):
            out_a.extend(F32.pack(x))
            out_b.extend(F32.pack(y))
        self.assertEqual(bytes(out_a), a['reference'])
        self.assertEqual(bytes(out_b), b['reference'])

    def test_invalid_prepared_controls_preserve_the_previous_values(self):
        valid = (.9, .95, .5, .99)
        for field in range(4):
            for bad in (float('nan'), float('inf'), -float('inf'), -.001, 1.001):
                p = Prepared(*valid)
                before = bytes(p)
                args = list(valid)
                args[field] = bad
                self.assertNotEqual(self.lib.dnbx_s1_envelope_configure(C.byref(p), *args), 0)
                self.assertEqual(bytes(p), before)
        self.assertNotEqual(self.lib.dnbx_s1_envelope_configure(None, *valid), 0)

    def test_reset_is_local_and_reproduces_initial_attack(self):
        case = self.cases[0]
        state, peer = self.new_state(), self.new_state()
        p = self.prepared(case['settings'])
        for _ in range(512):
            self.lib.dnbx_s1_envelope_sample(C.byref(state), C.byref(p), 1)
            self.lib.dnbx_s1_envelope_sample(C.byref(peer), C.byref(p), 1)
        before = bytes(peer)
        self.lib.dnbx_s1_envelope_reset(C.byref(state))
        self.assertEqual(bytes(peer), before)
        rendered = F32.pack(self.lib.dnbx_s1_envelope_sample(C.byref(state), C.byref(p), 1))
        self.assertEqual(rendered, case['reference'][:F32.size])

    def test_decay_edit_at_attack_boundary_is_latched_on_the_next_sample(self):
        # Exact binary fractions exercise our declared event-order invariant;
        # this adds component coverage, not another original-app recording.
        state = self.new_state()
        p = Prepared()
        self.assertEqual(self.lib.dnbx_s1_envelope_configure(C.byref(p), .5, .25, .5, .5), 0)
        for _ in range(7):
            value = self.lib.dnbx_s1_envelope_sample(C.byref(state), C.byref(p), 1)
        self.assertEqual(value, .9921875)
        self.assertEqual(self.lib.dnbx_s1_envelope_configure(C.byref(p), .5, .75, .5, .5), 0)
        value = self.lib.dnbx_s1_envelope_sample(C.byref(state), C.byref(p), 1)
        self.assertEqual(value, .869140625)


def retained_red(path):
    raw = Path(path).read_bytes()
    cases_by_id = {case['id']: case for case in fixtures()}
    selected = [cases_by_id['live-' + str(i)] for i in range(5)]
    expected_bytes = sum(c['frames'] for c in selected) * F32.size
    if len(raw) != expected_bytes:
        raise ValueError(f'Retained PCM length {len(raw)} does not match {expected_bytes}')
    rows, offset = [], 0
    for case in selected:
        size = case['frames'] * F32.size
        rows.append({'case': case['id'], **difference(case['reference'], raw[offset:offset + size])})
        offset += size
    print(json.dumps({'status': 'RETAINED_IMPLEMENTATION_FAIL',
                      'evidence': 'Comparing saved actual old-core C output; no old C execution claimed',
                      'candidate_sha256': hashlib.sha256(raw).hexdigest(), 'cases': rows}, indent=2))
    return 1 if any(row.get('different_words', 0) for row in rows) else 0


class FixtureIntegrityTest(unittest.TestCase):
    def test_metadata_changes_are_rejected_before_case_selection(self):
        with tempfile.TemporaryDirectory(prefix='dnbox-s1-fixture-guard-') as temp:
            static, live = Path(temp) / 'static', Path(temp) / 'live'
            static.mkdir()
            live.mkdir()
            for source, destination, names in (
                (STATIC, static, ('s1_envelope_observations.f32le', 'envelope_observations.json')),
                (LIVE, live, ('reference_observations.f32le', 'observations.json', 'case_plan.json')),
            ):
                for name in names:
                    (destination / name).write_bytes((source / name).read_bytes())
            for path in (static / 'envelope_observations.json', live / 'observations.json',
                         live / 'case_plan.json'):
                with self.subTest(metadata=path.name):
                    original = path.read_bytes()
                    path.write_bytes(b' ' + original)  # Still valid JSON; no longer the pinned input.
                    with self.assertRaisesRegex(AssertionError, 'reference identity changed'):
                        fixtures(static, live)
                    path.write_bytes(original)

    def test_truncated_retained_pcm_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix='dnbox-s1-retained-guard-') as temp:
            path = Path(temp) / 'empty.f32le'
            path.write_bytes(b'')
            with self.assertRaisesRegex(ValueError, 'PCM length'):
                retained_red(path)


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--retained-baseline':
        sys.exit(retained_red(sys.argv[2]))
    unittest.main()
