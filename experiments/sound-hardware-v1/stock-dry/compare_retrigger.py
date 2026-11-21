"""Compare back-to-back note-off/on against the authenticated original Synth One scheduler."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tarfile

import numpy as np

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
KEYS = (36, 60, 84)
ONSET = 300 * 64
WINDOW = slice(ONSET, ONSET + 8192)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def compile_sources(output, source_tree):
    original = output / 'original-inputs'
    build = output / 'original-build'
    subprocess.run(['python3', REPO / 'tools/restore-synthone-test-inputs.py', '--bundle',
                    REPO / 'assets/test-inputs/synthone', '--output', original], check=True)
    subprocess.run(['python3', REPO / 'tests/host/run_synthone_renderer.py',
                    '--source-build', original, '--output', build], check=True)
    with tarfile.open(HERE / 'part-gain-proof.tar.gz') as archive:
        def fixture(name):
            return archive.extractfile('part-gain/' + name).read()
        original_source = fixture('original.cpp').decode()
        candidate_source = fixture('candidate.c').decode()
        levels = json.loads(fixture('levels.json'))
    # Keep the original-only wrapper's input, tuning, and warmup checks. Add
    # exactly one adjacent off/on before its ordinary render for block 300.
    wrappers = [
        ('reference.cpp', original_source, 'if(b==kOffBlock)',
         'original.stopNote(notes[i]); original.startNote(notes[i],kVelocity);'),
        ('candidate.c', candidate_source, 'if(b==1033)',
         's1_part_note(&p,keys[i],0); s1_part_note(&p,keys[i],100);'),
    ]
    for name, source, insertion, edges in wrappers:
        if source.count(insertion) != 1:
            raise ValueError(f'Unrecognized capture loop in {name}')
        event = 'if (b == 300 && getenv("S1_RETRIGGER")) for(unsigned i=0;i<n;i++) {' + edges + '}'
        (output / name).write_text(source.replace(insertion, event + '\n        ' + insertion))

    compile_commands = json.loads((build / 'commands.json').read_text())
    oracle_command, = [cmd for cmd in compile_commands if any(arg.endswith('/full_oracle.cpp') for arg in cmd)]
    flags = oracle_command[:oracle_command.index('-c')]
    subprocess.run([*flags, '-Wno-deprecated', '-Wno-unused-const-variable',
                    '-c', output / 'reference.cpp', '-o', output / 'reference.o'], check=True)
    objects = [build / f'dependency-{i:03d}.o' for i in [*range(13), 14, 21, 22, 25]]
    subprocess.run(['c++', output / 'reference.o', *objects, '-Wl,--gc-sections',
                    '-lm', '-o', output / 'reference'], check=True)

    app = source_tree / 'firmware/src/app'
    dsp = source_tree / 'firmware/src/dsp'
    subprocess.run(['cc', '-std=c11', '-O2', '-ffp-contract=off', '-fno-fast-math',
                    '-DX0X_HOST', '-I' + str(app), '-I' + str(dsp), output / 'candidate.c',
                    app / 's1_dry_part.c', dsp / 's1_dry_voice.c', dsp / 'dnbx_s1_envelope.c',
                    '-lm', '-o', output / 'candidate'], check=True)
    manifest = json.loads((HERE.parent / 'dry_voice_followup/reference_reconstruction/metadata'
                           / 'numeric_fixture_manifest.json').read_text())
    for name, entry, key in [('controls', manifest['controls'][0], 'control_bits_hex'),
                             ('tuning', manifest['tuning'], 'bits_hex')]:
        data = b''.join(struct.pack('<I', int(word, 16)) for word in entry[key])
        if digest(data) != entry['input']['sha256']:
            raise ValueError(f'Numeric fixture differs: {name}')
        (output / name).write_bytes(data)
    return {row['keys'][0]: row['original_sha256'] for row in levels if len(row['keys']) == 1}


def compare(output, keys, hashes):
    rows = []
    for key in keys:
        deltas = {}
        captures = {}
        for name in ('reference', 'candidate'):
            waves = []
            for retrigger in (False, True):
                target = output / f'{name}-{key}-{int(retrigger)}.pcm'
                cmd = [output / name]
                if name == 'reference':
                    cmd.extend([output / 'original-inputs/public-morphs.f32', output / 'controls',
                                output / 'tuning', target, str(key)])
                else:
                    cmd.extend([str(key), target])
                env = os.environ.copy()
                env.pop('S1_RETRIGGER', None)
                if retrigger:
                    env['S1_RETRIGGER'] = '1'
                subprocess.run(cmd, env=env, check=True)
                data = target.read_bytes()
                captures[target.name] = digest(data)
                if name == 'reference' and not retrigger and digest(data) != hashes[key]:
                    raise ValueError(f'Original held-note capture differs at {key}')
                wave = np.frombuffer(data, '<f4').astype(float)
                if len(wave) != 88192 or not np.isfinite(wave).all():
                    raise ValueError(f'Invalid render for {target.name}')
                waves.append(wave)
            if not np.array_equal(waves[0][:ONSET], waves[1][:ONSET]):
                raise ValueError(f'Event preceded its scheduled frame for {name}/{key}')
            deltas[name] = (waves[1] - waves[0])[WINDOW]
        ref, actual = deltas['reference'], deltas['candidate']
        if not np.any(ref):
            raise ValueError(f'Original retrigger had no effect at {key}')
        relative = np.linalg.norm(actual - ref) / np.linalg.norm(ref)
        rows.append(dict(key=key, reference_peak=float(np.max(np.abs(ref))),
                         candidate_peak=float(np.max(np.abs(actual))),
                         relative_waveform_error_db=float(20 * np.log10(relative)), pcm_sha256=captures))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, help='fresh output directory')
    parser.add_argument('--candidate-tree', type=Path, default=REPO)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    hashes = compile_sources(output, args.candidate_tree.resolve())
    rows = compare(output, KEYS, hashes)
    (output / 'results.json').write_text(json.dumps(rows, indent=2) + '\n')
    for row in rows:
        print(f"MIDI {row['key']}: {row['relative_waveform_error_db']:.3f} dB relative waveform error")
    # The measured change must be closer to the original than omitting it.
    return int(any(row['relative_waveform_error_db'] >= 0 for row in rows))


if __name__ == '__main__':
    raise SystemExit(main())
