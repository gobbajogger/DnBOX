#!/usr/bin/env python3
"""Measure the optional firmware's factory input coverage. Output is diagnostic, always unscored."""
import argparse
from hashlib import sha256
import json
import math
from pathlib import Path
import struct
import subprocess
import sys


def digest(data):
    return sha256(data).hexdigest()


def word(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]


def write(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def run(binary, words, spec, directory, name):
    pcm = directory / (name + '.partial.f32le')
    events = spec['events']
    commands = [f"{len(words)} {spec['capture_frames']} {spec['block_frames']} {len(events)}"]
    commands += [f'{ident} {bits:08x}' for ident, bits in words]
    for event in events:
        if event['action'] not in ('on', 'off') or bool(event['velocity']) != (event['action'] == 'on'):
            raise ValueError('unrepresentable event')
        commands.append(f"{event['frame']} {event['note']} {event['velocity']}")
    proc = subprocess.run([binary, pcm], input='\n'.join(commands)+'\n', text=True,
                          capture_output=True, check=True)
    raw, layers = {}, {}
    state = {}
    for line in proc.stdout.splitlines():
        fields = line.split()
        if not fields:
            continue
        tag, tail = fields[0], fields[1:]
        if tag == 'stored':
            raw[int(tail[0])] = int(tail[1], 16)
        elif tag == 'engine':
            state['selected_dnbox_bank'], state['selected_dnbox_preset'] = map(int, tail[:2])
            state['clock_tempo_binary32'] = tail[2]
        elif tag == 'layer':
            layers[tail[0]] = dict(zip(('oscillator_mode', 'waveform', 'pulse_width', 'tune_semitones',
                'amp_attack_s', 'amp_decay_s', 'amp_sustain', 'amp_release_s'), tail[1:], strict=True))
        elif tag == 'master':
            state['physical_output_gain_binary32'] = tail[0]
        elif tag == 'chorus':
            state['dnbox_chorus_binary32'] = dict(zip(('rate_hz', 'depth_ms', 'mix'), tail, strict=True))
    if len(layers) != 2 or 'physical_output_gain_binary32' not in state or 'dnbox_chorus_binary32' not in state or not raw:
        raise ValueError('incomplete probe output')
    data = pcm.read_bytes()
    if len(data) != spec['capture_frames'] * 8: raise ValueError('truncated diagnostic PCM')
    nonzero = [0, 0]
    stereo_differences = 0
    for left, right in struct.iter_unpack('<ff', data):
        if not (math.isfinite(left) and math.isfinite(right)): raise ValueError('nonfinite diagnostic PCM')
        nonzero[0] += left != 0.0
        nonzero[1] += right != 0.0
        stereo_differences += left != right
    state['layers'] = layers
    result = dict(qualified=False, capture_point='host original_runtime_render after DnBOX chorus and instrument master',
                  live_dnbox=state, stored_binary32=raw, frames=spec['capture_frames'],
                  pcm_sha256=digest(data), nonzero_by_channel=nonzero, unequal_stereo_frames=stereo_differences)
    write(directory / (name + '.partial.json'), result)
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('tree', type=Path)
    ap.add_argument('output', type=Path)
    group = ap.add_mutually_exclusive_group()
    group.add_argument('--inputs', type=Path, help='factory.py matrix output, authenticated before use')
    group.add_argument('--release-ipa', '--release-app', dest='release_source', type=Path,
                       help='authenticated original build-9 IPA, installed .app or embedded .appex')
    ap.add_argument('--self-test', action='store_true')
    args = ap.parse_args()
    exp = args.tree.resolve() / 'experiments/sound-hardware-v1/sj6-factory'
    sys.path.insert(0, str(exp))
    from factory import canonical_bytes
    if args.release_source:
        from release_matrix import prepare_release
        spec_raw, audit, files = prepare_release(args.release_source)
    else:
        spec_raw = (exp / 's4-inputs.json').read_bytes()
        audit = json.loads((exp / 'original/bundled-bank-audit.json').read_bytes())
        files = None if not args.inputs else {
            row['uid']: (args.inputs / (row['uid']+'.json')).read_bytes() for row in audit['programs']}
    if digest(spec_raw) != audit['spec_sha256']: raise ValueError('audit/spec digest mismatch')
    spec = json.loads(spec_raw)
    catalog = json.loads((exp.parent / 'mapping/results/implementer_brief/public_control_catalog.json').read_bytes())
    ids = {p['identifier']: p['id'] for p in catalog['controls']}
    for key in sorted(catalog['unknown_legacy_keys']): ids[key] = len(ids)
    binary = args.output / 'build/test'
    proc = subprocess.run([binary, '--schema'], text=True, capture_output=True, check=True)
    routed = {int(line.split()[1]) for line in proc.stdout.splitlines() if line.startswith('accepted ')}
    if not routed: raise ValueError('firmware row admission returned no fields')
    rows = {p['uid']: p for p in audit['programs']}
    output = args.output
    report = dict(qualified=False, host_executable_sha256=digest(binary.read_bytes()),
                  spec_sha256=digest(spec_raw), feature='X0X_SUPERJ6_FACTORY=1',
                  firmware_transaction_ids=sorted(routed), programs=[])
    for row in spec['programs']:
        uid = row['program_uid']
        checked = rows[uid]
        if checked['supplied_bank_uuid'] != row['bank_uuid']: raise ValueError('bank mismatch')
        n = checked['parameter_count']
        entry = dict(uid=uid, name=row['program'], authored=n, minimum_unrouted=max(0,n-len(routed)))
        if files is not None:
            data = files[uid]
            if digest(data) != checked['file_sha256']: raise ValueError('input file digest mismatch')
            record = json.loads(data)
            preset = record['preset']
            if digest(canonical_bytes(preset)) != row['record_sha256'] or preset['uid'] != uid or record['bank']['uuid'] != row['bank_uuid']:
                raise ValueError('input record does not match authenticated identity')
            fields = [(ids[p['identifier']], word(p['value'])) for p in preset['parameterValues']]
            if len(fields) != n or len(dict(fields)) != n: raise ValueError('unexpected control count')
            state = run(binary, fields, spec, output, uid)
            authored = dict(fields)
            entry.update(unrouted_ids=sorted(authored.keys() - routed),
                         stored_matches=[k for k in authored if state['stored_binary32'].get(k) == authored[k]],
                         diagnostic_pcm_sha256=state['pcm_sha256'])
        report['programs'].append(entry)
    if args.self_test:
        # All actual original-control identifiers supplied to ingress. Only the test vector values are synthetic.
        values = {n: word(.25) for n in ids.values()}
        values.update({0: word(.75), 82: word(1), 83: word(.75), 86: word(.5),
                       119: word(.05), 120: word(.1), 121: word(.8), 122: word(.2)})
        first = run(binary, list(values.items()), spec, output, 'fixture-base')
        changed = dict(values)
        for i in values.keys() - routed: changed[i] = word(.875)
        ignored = run(binary, list(changed.items()), spec, output, 'fixture-unrouted')
        if first != ignored or first['stored_binary32'].keys() != routed:
            raise ValueError('unrouted change affected state or output')
        if first['stored_binary32'] != {i: values[i] for i in routed}:
            raise ValueError('accepted words differ from real stored controls')
        if min(first['nonzero_by_channel']) == 0 or not first['unequal_stereo_frames']:
            raise ValueError('fixture did not exercise both channels')
        changed[0] = word(0)
        silent = run(binary, list(changed.items()), spec, output, 'fixture-zero-master')
        if silent['live_dnbox']['physical_output_gain_binary32'] != '00000000' or silent['nonzero_by_channel'] != [0,0]:
            raise ValueError('real zero master did not silence both channels')
        if silent['pcm_sha256'] == first['pcm_sha256']: raise ValueError('missing positive-control difference')
        report['synthetic_probe'] = dict(changed_unrouted_controls=len(values.keys()-routed),
            unchanged_state_and_stereo_sha256=first['pcm_sha256'], unequal_stereo_frames=first['unequal_stereo_frames'],
            zero_master_sha256=silent['pcm_sha256'], frames_each=first['frames'], events=len(spec['events']))
    write(output / 'candidate-ingress.json', report)
    print('UNSCORED', len(rows), 'rows;', len(routed), 'identifiers admitted by the factory transaction')
    if args.self_test: print(json.dumps(report['synthetic_probe'], sort_keys=True))


if __name__ == '__main__':
    main()
