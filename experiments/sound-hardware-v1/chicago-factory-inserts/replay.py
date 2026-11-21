"""Compare the selected Chicago insert with the archived full-history Hall and BPM delay."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import numpy as np


def source_headers(repo, shadow):
    code = repo / 'experiments/original_synth'
    target = shadow / 'experiments/original_synth'
    target.mkdir(parents=True)
    (shadow / 'firmware').symlink_to(repo / 'firmware', target_is_directory=True)
    for p in code.glob('*.h'):
        if p.name not in ('chicago_fx2_compact.h', 'chicago_fx2_inserts.h'):
            (target / p.name).symlink_to(p)
    pinned = 'bf48ce83bea6f8d332dddff11e5407c4fd744fc6'
    def baseline(name):
        return subprocess.check_output(['git', '-C', repo, 'show',
                                        pinned + ':experiments/original_synth/' + name], text=True)
    header = baseline('chicago_fx2_compact.h')
    width = 'ORIGINAL_CHICAGO_FX2_COMPACT_HISTORY_SAMPLES = 2048'
    assert header.count(width) == 1
    (target / 'chicago_fx2_compact.h').write_text(header.replace(width, 'ORIGINAL_CHICAGO_FX2_COMPACT_HISTORY_SAMPLES = (sizeof(chi_fx2_insert_arena) + 1) / 2'))
    h = baseline('chicago_fx2_inserts.h')
    h = h.replace('chi_hall_q_t hall;', 'chi_hall_t hall;').replace('chi_bpm2_t bpm;', 'chi_bpmdly_t bpm;')
    h = h.replace('a->hall.damp = 0.42f; a->hall.lvl = 0.67f; break;', 'chi_hall_init(&a->hall); break;')
    h = h.replace('default: a->bpm.delay = 512.0f; break;', 'default: chi_bpmdly_init(&a->bpm); break;')
    h = h.replace('chi_bpm2_set(&a->bpm, e1, e2, bpm_times10)', 'chi_bpmdly_set_bpm(&a->bpm, e1, e2, CHI_FX2_SR, (float)bpm_times10 * .1f)')
    start = h.index('    case 15: {')
    stop = h.index('    case 16:', start)
    h = h[:start] + '    case 15: chi_hall_set(&a->hall, e1, e2, CHI_FX2_SR); break;\n' + h[stop:]
    h = h.replace('chi_bpm2_process(&a->bpm', 'chi_bpmdly_process(&a->bpm').replace('chi_hall_q_process(&a->hall', 'chi_hall_process(&a->hall')
    (target / 'chicago_fx2_inserts.h').write_text(h)
    return target / 'chicago_fx2_compact.h'


def compile_renderer(repo, executable, flags, build):
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-O2', '-D_POSIX_C_SOURCE=200809L',
                    '-DX0X_HOST', '-DX0X_ORIGINAL_SYNTH=1', '-DX0X_DEV_WOBBLE=0', '-DX0X_WOBBLE=0',
                    '-Ifirmware/src', '-Ifirmware/src/dsp', '-Ibuild/gen', '-Iexperiments/original_synth',
                    '-Iexperiments/wobble', *flags, *build.ORIGINAL_SYNTH_FLAGS,
                    'tests/host/bass_reference_render.c', *build.ORIGINAL_SYNTH_UNITS,
                    *build.ORIGINAL_SYNTH_BACKENDS, 'firmware/src/dsp/x0x_param.c', '-lm', '-o', executable],
                   cwd=repo, check=True)


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('repo', type=Path)
    cli.add_argument('output', type=Path)
    cli.add_argument('--full-history', action='store_true',
                     help='Enable the native Elite/Pro profile for the firmware arm')
    args = cli.parse_args()
    repo, out = args.repo.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    sys.path.insert(0, str(repo / 'tests/host'))
    from bass_factory_effects import CHICAGO_INSERTS, factory_render, digest
    from bass_reference_qualification import build, check_original_inputs
    scorer_path = repo / 'experiments/sound-hardware-v1/sj6-factory/compare.py'
    sys.path.insert(0, str(scorer_path.parent))
    spec = importlib.util.spec_from_file_location('factory_channel_score', scorer_path)
    scorer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(scorer)
    channel_score = scorer.channel_score
    if not check_original_inputs.verify(repo, expected_manifest_sha256=check_original_inputs.MANIFEST_SHA256)['ok']:
        raise ValueError('Original bass source input verification failed')
    ordinary = out / 'firmware'
    ref = out / 'archived'
    adapted = source_headers(repo, out / 'original-source')
    compile_renderer(repo, ordinary, ['-DX0X_CHICAGO_FULL_HISTORY=' + str(int(args.full_history))], build)
    compile_renderer(repo, ref, ['-include', adapted], build)
    bank = json.loads((repo / 'assets/factory_banks/chicago.json').read_bytes())['presets']
    rows = []
    for scenario in sorted((repo / 'tests/scenarios/chicago_au').glob('chicago_au_*.json')):
        scene = json.loads(scenario.read_bytes())
        program = bank[scene['factoryPresetIndex']]
        assert program['name'] == scene['factoryPresetName']
        effect = CHICAGO_INSERTS[round(program['params'][17] * 11)]
        archive = factory_render(ref, 'chicago', scene, 'stored')
        candidate = factory_render(ordinary, 'chicago', scene, 'stored')
        dry = factory_render(ordinary, 'chicago', scene, 'bypass')
        assert np.all(np.any(archive != 0, axis=0)) and np.all(np.any(candidate != 0, axis=0))
        assert np.isfinite(archive).all() and np.isfinite(candidate).all()
        row = dict(index=scene['factoryPresetIndex'], name=program['name'], effect=effect,
                   frames=len(archive), archived_sha256=digest(archive), firmware_sha256=digest(candidate),
                   bypass_sha256=digest(dry))
        for name, a, b in [('full', archive, candidate), ('difference_from_bypass', archive - dry, candidate - dry)]:
            active = bool(np.all(np.any(a != 0, axis=0)) and np.all(np.any(b != 0, axis=0)))
            scores = [channel_score(a[:,i], b[:,i], 44100, [20,16000]) for i in (0,1)] if active else []
            row[name] = dict(active=active, exact=bool(np.array_equal(a,b)), channels=scores,
                             passed=active and all(abs(level)<=1 and spec<=3 for level,spec in scores))
        rows.append(row)
    assert len(rows) == 81
    (out / 'scores.json').write_text(json.dumps(rows, indent=2) + '\n')
    for name in ('full', 'difference_from_bypass'):
        print(name, sum(r[name]['passed'] for r in rows), '/', sum(r[name]['active'] for r in rows))


if __name__ == '__main__':
    main()
