#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Plan the authenticated SuperJ6 factory schedule and package verified original captures."""
import argparse
import hashlib
from io import BytesIO
from pathlib import Path
import re
from tempfile import TemporaryDirectory
from zipfile import ZIP_DEFLATED, ZipFile

import numpy as np

from compare import capture, channel_score, control_words, digest, expect, json_object
from factory import canonical_bytes
from original.audit_bundled_banks import AU, IPA, PREFIX
from release_matrix import EXECUTABLE, HERE, json_bytes, select_bundled

SUFFIXES = ('.f32le', '.json', '.applied.json')
UID_RE = re.compile(r'[0-9A-Fa-f]{8}(?:-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}')
SCHEDULE_KEYS = ('sample_rate', 'channels', 'capture_frames', 'block_frames',
                 'reset', 'transport', 'gain', 'events', 'band_hz',
                 'level_limit_db', 'spectral_limit_db')


def factory_matrix(banks, schedule, audit, component_version):
    """Create a separate 640-case plan from the release containers, not an export."""
    factory_banks = audit['banks']
    expect(len(factory_banks), 12, 'factory bank count')
    expect(len({b['name'] for b in factory_banks}), 12, 'unique factory bank names')
    expect(sorted(banks),
           sorted([b['name'] for b in factory_banks] + ['User Bank']), 'release bank names')
    for extra in audit['unpaired_programs']:
        if extra['present_only_in'] != 'release' or extra['bank'] != 'User Bank':
            raise ValueError('unexpected unpaired release program')
    expect(sorted(p['uid'] for p in banks['User Bank']['presets']),
           sorted(p['uid'] for p in audit['unpaired_programs']), 'release-only bank')

    spec = {k: schedule[k] for k in SCHEDULE_KEYS}
    spec.update(scope='superj6-factory-640', source_ipa_sha256=IPA, programs=[])
    inputs, checks, total_controls = {}, [], 0
    for expected in factory_banks:
        bank = banks[expected['name']]
        expect(bank['uuid'], expected['release_bank_uuid'], 'release bank UUID')
        expect(len(bank['presets']), expected['release_programs'], 'release bank size')
        metadata = {k: v for k, v in bank.items() if k != 'presets'}
        for preset in bank['presets']:
            uid = preset['uid']
            if not UID_RE.fullmatch(uid) or uid in inputs:
                raise ValueError('invalid or duplicate factory UID')
            inputs[uid] = json_bytes({'bank': metadata, 'preset': preset})
            words = len(control_words(preset['parameterValues']))
            total_controls += words
            spec['programs'].append({
                'bank': bank['name'], 'bank_uuid': bank['uuid'], 'program': preset['name'],
                'program_uid': uid, 'position': preset['position'],
                'record_sha256': digest(canonical_bytes(preset)),
            })
            checks.append({'uid': uid, 'bundled_bank_uuid': bank['uuid'],
                           'supplied_bank_uuid': bank['uuid'], 'parameter_count': words,
                           'file_sha256': digest(inputs[uid])})
    expect(len(inputs), 640, 'factory program count')
    expect(total_controls, 90549, 'factory control count')
    raw = json_bytes(spec)
    return raw, {'ipa_sha256': IPA, 'au_sha256': AU, 'component_version': component_version,
                 'spec_sha256': digest(raw), 'programs': checks}, inputs


def prepare(ipa):
    raw = ipa.read_bytes()
    expect(digest(raw), IPA, 'pinned build-9 IPA')
    banks = {}
    with ZipFile(BytesIO(raw)) as archive:
        expect(digest(archive.read(EXECUTABLE)), AU, 'original executable')
        for name in archive.namelist():
            if name.startswith(PREFIX) and name.endswith('.superj6.bank.json'):
                bank = json_object(archive.read(name))
                if bank['name'] in banks:
                    raise ValueError('duplicate release bank name')
                banks[bank['name']] = bank
    schedule = (HERE/'s4-inputs.json').read_bytes()
    audit = json_object((HERE/'original/all-programs.json').read_bytes())
    s4_audit = json_object((HERE/'original/bundled-bank-audit.json').read_bytes())
    expect(audit['ipa_sha256'], IPA, 'factory audit IPA')
    expect(audit['au_sha256'], AU, 'factory audit executable')
    s4 = select_bundled({b['uuid']: b for b in banks.values()}, schedule, s4_audit)
    expect(len(json_object(s4[0])['programs']), 14, 'S4 coverage')
    batch = factory_matrix(banks, json_object(schedule), audit, s4[1]['component_version'])
    return batch, s4


def batch_slice(spec, start, count):
    if start < 0 or count < 1 or start + count > len(spec['programs']):
        raise ValueError('batch must be a nonempty contiguous subset of the plan')
    return spec['programs'][start:start+count]


def originals_for_batch(batch, original, reuse=None, start=0, count=640, snapshot=None):
    raw, audit, inputs = batch
    spec = json_object(raw)
    selected = batch_slice(spec, start, count)
    s4_rows = {}
    if reuse:
        s4_path, (s4_raw, s4_audit, s4_inputs) = reuse
        s4_spec = json_object(s4_raw)
        expect(len(s4_spec['programs']), 14, 'S4 coverage')
        expect({k: s4_spec[k] for k in SCHEDULE_KEYS}, {k: spec[k] for k in SCHEDULE_KEYS}, 'S4 stimulus')
        s4_rows = {r['program_uid']: r for r in s4_spec['programs']}
        s4_sha = digest(s4_raw)
    batch_sha = digest(raw)
    for row in selected:
        uid = row['program_uid']
        filenames = [uid + suffix for suffix in SUFFIXES]
        directory, scope = original, 'factory'
        rec = original/(uid+'.json')
        # Unpacked archives can include both matrix receipts. Require explicit
        # reuse, then validate the original receipt against its own matrix.
        is_archived_s4 = (uid in s4_rows and original.resolve() == s4_path.resolve() and rec.exists() and
                          json_object(rec.read_bytes()).get('s4_inputs_sha256') == s4_sha)
        missing = not any((original/name).exists() for name in filenames)
        if uid in s4_rows and (missing or is_archived_s4):
            directory, scope = s4_path, 's4'
            s4_row = s4_rows[uid]
            expect({k: s4_row[k] for k in row}, row, 'S4 program identity')
            if s4_inputs[uid] != inputs[uid]:
                raise ValueError('S4 input record bytes differ')
        if snapshot is not None:
            for name in filenames:
                (snapshot/name).write_bytes((directory/name).read_bytes())
            directory = snapshot
        if scope == 's4':
            _, pcm_sha = capture(directory, s4_row, s4_spec, s4_sha, 'original',
                                 s4_inputs, audit=s4_audit)
            yield row, directory, pcm_sha, scope, s4_sha
        else:
            _, pcm_sha = capture(directory, row, spec, batch_sha, 'original', inputs, audit=audit)
            yield row, directory, pcm_sha, scope, batch_sha
        if snapshot is not None:
            for name in filenames:
                (snapshot/name).unlink()


def verify_archive(path, manifest):
    with ZipFile(path) as archive:
        names = [item['path'] for item in manifest['files']]
        expect(sorted(archive.namelist()), sorted(names + ['manifest.json']), 'archive members')
        expect(json_object(archive.read('manifest.json')), manifest, 'archive manifest')
        for item in manifest['files']:
            raw = archive.read(item['path'])
            expect(len(raw), item['bytes'], 'unpacked byte count')
            expect(digest(raw), item['sha256'], 'unpacked digest')


def file_sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def verify_published_archive(path, expected_sha):
    """Verify the published outer digest and every decompressed contract member."""
    expect(file_sha(path), expected_sha, 'archive digest')
    with ZipFile(path) as archive:
        manifest = json_object(archive.read('manifest.json'))
    expect(manifest.get('format'), 'superj6-original-stereo-fixtures-v1', 'archive format')
    expect(manifest.get('arm'), 'original', 'archive source arm')
    expect(manifest.get('scope'), 'superj6-factory-640', 'archive scope')
    programs = manifest['programs']
    if not programs or len(programs) > 640:
        raise ValueError('invalid batch coverage')
    names = set()
    by_name = {f['path']: f for f in manifest['files']}
    schedule = manifest['schedule']
    expect((schedule['sample_rate'], schedule['channels'], schedule['capture_frames']),
           (44100, 2, 132300), 'archive PCM profile')
    for program in programs:
        uid = program['program_uid']
        if not UID_RE.fullmatch(uid) or uid+'.json' in names:
            raise ValueError('invalid or duplicate packaged UID')
        names.update(uid+s for s in SUFFIXES)
        pcm = by_name[uid+'.f32le']
        expect(pcm['sha256'], program['pcm_sha256'], 'program PCM digest')
        expect(pcm['bytes'], schedule['capture_frames'] * schedule['channels'] * 4, 'program PCM length')
    expect(sorted(names), sorted(f['path'] for f in manifest['files']), 'capture contract filenames')
    verify_archive(path, manifest)
    return {'programs': len(programs), 'archive_sha256': expected_sha, 'archive_bytes': path.stat().st_size,
            'unpacked_payload_bytes': sum(f['bytes'] for f in manifest['files'])}


def pack(batch, original, output, reuse=None, start=0, count=640):
    if output.exists():
        raise FileExistsError(output)
    raw, audit, _ = batch
    spec = json_object(raw)
    manifest = {'format': 'superj6-original-stereo-fixtures-v1', 'arm': 'original',
                'source_ipa_sha256': audit['ipa_sha256'], 'au_sha256': audit['au_sha256'],
                'scope': spec['scope'], 'schedule_sha256': digest(raw), 'start': start,
                'schedule': {k: spec[k] for k in SCHEDULE_KEYS}, 'programs': [], 'files': []}
    with TemporaryDirectory(dir=output.parent) as tmp:
        temp = Path(tmp)/'batch.zip'
        snapshot = Path(tmp)/'capture'
        snapshot.mkdir()
        with ZipFile(temp, 'w', ZIP_DEFLATED, compresslevel=6) as archive:
            for row, directory, pcm_sha, scope, matrix_sha in originals_for_batch(batch, original, reuse, start, count, snapshot):
                uid = row['program_uid']
                manifest['programs'].append(dict(row, pcm_sha256=pcm_sha, receipt_scope=scope,
                                                 receipt_matrix_sha256=matrix_sha))
                for suffix in SUFFIXES:
                    name = uid + suffix
                    data = (directory/name).read_bytes()
                    if suffix == '.f32le':
                        expect(digest(data), pcm_sha, 'packed PCM digest')
                    archive.writestr(name, data)
                    manifest['files'].append({'path': name, 'bytes': len(data), 'sha256': digest(data)})
            archive.writestr('manifest.json', json_bytes(manifest))
        verify_archive(temp, manifest)
        # Link rather than replace so concurrent writers cannot overwrite a batch.
        output.hardlink_to(temp)
    return {'programs': len(manifest['programs']), 'archive_bytes': output.stat().st_size,
            'archive_sha256': file_sha(output),
            'unpacked_payload_bytes': sum(f['bytes'] for f in manifest['files'])}


def score_batch(batch, original, dnbox, image, commit, start, count, reuse=None):
    raw, audit, inputs = batch
    spec = json_object(raw)
    image_bytes = image.read_bytes()
    if not image_bytes:
        raise ValueError('empty production image')
    image_sha = digest(image_bytes)
    matrix_sha = digest(raw)
    results = []
    for row, directory, a_sha, scope, original_matrix in originals_for_batch(batch, original, reuse, start, count):
        pcm = (directory/(row['program_uid']+'.f32le')).read_bytes()
        expect(digest(pcm), a_sha, 'original PCM digest')
        a = np.frombuffer(pcm, '<f4').reshape(-1, spec['channels'])
        b, b_sha = capture(dnbox, row, spec, matrix_sha, 'dnbox', inputs,
                           candidate_sha=image_sha, candidate_commit=commit, audit=audit)
        scores = [channel_score(x, y, spec['sample_rate'], spec['band_hz']) for x, y in zip(a.T, b.T)]
        passed = all(level is not None and abs(level) <= spec['level_limit_db'] and
                     spectral is not None and spectral <= spec['spectral_limit_db']
                     for level, spectral in scores)
        results.append({'uid': row['program_uid'], 'original_pcm_sha256': a_sha,
                        'original_receipt_scope': scope, 'original_receipt_matrix_sha256': original_matrix,
                        'dnbox_pcm_sha256': b_sha, 'channels': scores, 'passed': passed})
    return {'scope': spec['scope'], 'schedule_sha256': matrix_sha, 'source_commit': commit,
            'firmware_sha256': image_sha, 'programs': results, 'passed': sum(r['passed'] for r in results)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ipa', type=Path, help='pinned original build-9 IPA, used as data')
    sub = parser.add_subparsers(dest='command', required=True)
    plan = sub.add_parser('plan')
    plan.add_argument('output', type=Path)
    build = sub.add_parser('pack')
    build.add_argument('captures', type=Path)
    build.add_argument('output', type=Path)
    build.add_argument('--reuse-s4', type=Path)
    build.add_argument('--start', type=int, default=0)
    build.add_argument('--count', type=int, default=640)
    score = sub.add_parser('score')
    score.add_argument('original', type=Path)
    score.add_argument('dnbox', type=Path)
    score.add_argument('--reuse-s4', type=Path)
    score.add_argument('--candidate-image', required=True, type=Path)
    score.add_argument('--candidate-commit', required=True)
    score.add_argument('--start', type=int, default=0)
    score.add_argument('--count', type=int, default=640)
    verify = sub.add_parser('verify')
    verify.add_argument('archive', type=Path)
    verify.add_argument('--sha256', required=True)
    args = parser.parse_args()
    if args.command == 'verify':
        print(json_bytes(verify_published_archive(args.archive, args.sha256)).decode(), end='')
        return
    if args.ipa is None:
        parser.error('--ipa is required to plan, score or package original captures')
    batch, s4 = prepare(args.ipa)
    if args.command == 'plan':
        args.output.mkdir()
        raw, audit, _ = batch
        (args.output/'factory-inputs.json').write_bytes(raw)
        (args.output/'factory-audit.json').write_bytes(json_bytes(audit))
        return
    reuse = (args.reuse_s4, s4) if args.reuse_s4 else None
    if args.command == 'score':
        result = score_batch(batch, args.original, args.dnbox, args.candidate_image,
                             args.candidate_commit, args.start, args.count, reuse)
        print(json_bytes(result).decode(), end='')
        raise SystemExit(0 if result['passed'] == args.count else 1)
    print(json_bytes(pack(batch, args.captures, args.output, reuse, args.start, args.count)).decode(), end='')


if __name__ == '__main__':
    main()
