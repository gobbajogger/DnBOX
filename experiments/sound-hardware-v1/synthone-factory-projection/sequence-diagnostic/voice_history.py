#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Isolate recycled voice histories using the verified sequence replay output."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
VOICE = 'firmware/src/app/s1_dry_part.c'
SOURCE = 'f6c2f17087dea9de52dc77f88dd271aab6692ef1'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def replace_once(text, before, after):
    assert text.count(before) == 1, before
    return text.replace(before, after)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sequence_replay', type=Path)
    parser.add_argument('panel_targets', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    fixture, targets, output = [
        path.resolve() for path in (args.sequence_replay, args.panel_targets, args.output)
    ]
    output.mkdir()
    spec = importlib.util.spec_from_file_location('scorer', HERE.parent / 'replay.py')
    scorer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(scorer)
    baseline = json.loads((HERE/'focused-results.json').read_text())
    assert (fixture/'focused-results.json').read_bytes() == (HERE/'focused-results.json').read_bytes()
    assert sha(targets) == 'd72756ce2b5898b275e65f5b301527f2e687eb7df6c1c0ea0a8c3d5967237833'
    command = json.loads((fixture/'commands.json').read_text())['compile_candidate']
    for name in command:
        if name.startswith(('firmware/', 'experiments/')) and name.endswith('.c'):
            exact = subprocess.check_output(['git', 'show', f'{SOURCE}:{name}'], cwd=ROOT)
            assert (ROOT/name).read_bytes() == exact, name
    source = (ROOT/VOICE).read_text()
    fixture_source = (fixture/'shipping.c').read_text()
    for mode in ('phase', 'filter'):
        prior, restore = {
            'phase': ('    const float old_phase = voice->phase;\n', '    voice->phase = old_phase;\n'),
            'filter': ('    float old_filter[5];\n    memcpy(old_filter, voice->filter_history, sizeof old_filter);\n',
                       '    memcpy(voice->filter_history, old_filter, sizeof old_filter);\n'),
        }[mode]
        probe = replace_once(source, '    s1_dry_reset(voice);\n', prior + '    s1_dry_reset(voice);\n')
        probe = replace_once(probe, '    part->next = (slot + 1u) % S1_CORE_VOICES;',
                             restore + '    part->next = (slot + 1u) % S1_CORE_VOICES;')
        path = output/f'{mode}.c'
        path.write_text(probe)
        cmd = command.copy()
        cmd[cmd.index(VOICE)] = str(path)
        cmd[cmd.index(str(fixture/'shipping.c'))] = str(output/'shipping.c')
        cmd[-1] = str(output/mode)
        (output/'shipping.c').write_text(fixture_source)
        subprocess.run(cmd, check=True, cwd=ROOT)
    rows = []
    for row in baseline['rows']:
        i = row['index']
        inputs = fixture/str(i)
        oracle = inputs/'original.f32'
        events = inputs/'events.i32'
        assert sha(oracle) == row['original_sha256'] and sha(events) == row['event_sha256']
        ref = np.fromfile(oracle, dtype='<f4').reshape(-1, 2)
        result = {'index': i, 'original_sha256': sha(oracle), 'events_sha256': sha(events), 'arms': {}}
        for schedule, name in ((0, 'direct'), (2, 'oracle_events')):
            for arm in ('baseline', 'phase', 'filter'):
                binary = (fixture/'shipping') if arm == 'baseline' else (output/arm)
                pcm, panel = output/'pcm.f32', output/'panel.u8'
                subprocess.run([binary, targets, str(i), pcm, panel, str(schedule), events], check=True)
                assert panel.read_bytes() == (inputs/'direct.u8').read_bytes()
                if arm == 'baseline':
                    assert sha(pcm) == row['modes'][name]['sha256']
                audio = np.fromfile(pcm, dtype='<f4').reshape(-1, 2)
                scores = [scorer.score(ref[:, c], audio[:, c]) for c in (0, 1)]
                result['arms'][f'{name}:{arm}'] = {
                    'sha256': sha(pcm), 'channels': scores, 'passed': all(s['pass'] for s in scores)
                }
        rows.append(result)
    (output/'result.json').write_text(json.dumps({'source': SOURCE, 'rows': rows}, indent=2)+'\n')


if __name__ == '__main__':
    main()
