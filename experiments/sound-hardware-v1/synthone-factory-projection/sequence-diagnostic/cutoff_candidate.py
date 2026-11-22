#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Apply candidate cutoff modulation to the ten baseline guards and nine matched rows."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
SOURCE = 'f6c2f17087dea9de52dc77f88dd271aab6692ef1'
VOICE = 'firmware/src/app/s1_dry_part.c'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('sequence_replay', type=Path)
    ap.add_argument('panel_targets', type=Path)
    ap.add_argument('factory_build', type=Path)
    ap.add_argument('output', type=Path)
    args = ap.parse_args()
    fixture, targets, build, out = [p.resolve() for p in
        (args.sequence_replay, args.panel_targets, args.factory_build, args.output)]
    out.mkdir()
    spec = importlib.util.spec_from_file_location('scorer', HERE.parent/'replay.py')
    scorer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(scorer)
    assert (fixture/'focused-results.json').read_bytes() == (HERE/'focused-results.json').read_bytes()
    assert sha(targets) == 'd72756ce2b5898b275e65f5b301527f2e687eb7df6c1c0ea0a8c3d5967237833'
    a = (fixture/'shipping.c').read_text()
    assert a.count('static void key(') == 1 and a.count('assert(mode <= 2);') == 1
    a = a.replace('static void key(', 'unsigned s1_cutoff_probe_route;\nstatic void key(')
    a = a.replace('assert(mode <= 2);', 'assert(mode <= 2);\n    s1_cutoff_probe_route = (unsigned)strtoul(getenv("S1_PROBE_ROUTE"), NULL, 10);')
    (out/'shipping.c').write_text(a)
    cmd = json.loads((fixture/'commands.json').read_text())['compile_candidate']
    for name in cmd:
        if name.startswith(('firmware/', 'experiments/')) and name.endswith('.c'):
            raw = subprocess.check_output(['git', 'show', f'{SOURCE}:{name}'], cwd=ROOT)
            assert (ROOT/name).read_bytes() == raw, name
    source = (ROOT/VOICE).read_text()
    site = 'void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames)'
    assert source.count(site) == 1
    source = source.replace(site, (HERE/'cutoff_probe.inc').read_text()+'\n'+site)
    site = '    if (((pitch_route & 1u) && part->lfo[0].level != 0.f) ||'
    assert source.count(site) == 1
    source = source.replace(site, '''    if (s1_cutoff_probe_route && part->lfo[s1_cutoff_probe_route - 1u].level != 0.f)
        cutoff_probe_mix(part, left, frames, s1_cutoff_probe_route);
    else if (((pitch_route & 1u) && part->lfo[0].level != 0.f) ||''')
    (out/'voice.c').write_text(source)
    cmd[cmd.index(str(fixture/'shipping.c'))] = str(out/'shipping.c')
    cmd[cmd.index(VOICE)] = str(out/'voice.c')
    cmd[-1] = str(out/'shipping')
    subprocess.run(cmd, check=True, cwd=ROOT)
    with gzip.open(ROOT/'evidence/sound-resume/synthone-pitch-lfo/factory-40.jsonl.gz', 'rt') as rows:
        baseline = {r['index']:r for r in map(json.loads, rows)}
    focus = {r['index']:r for r in json.loads((HERE/'focused-results.json').read_text())['rows']}
    guards = {i for i,r in baseline.items() if r['pass']}
    assert len(guards) == 10
    assets = build/'factory-inputs'
    bank = build/'commands/preset-commands.bin'
    bank_bytes = bank.read_bytes()
    result = []
    for i in sorted(guards | focus.keys()):
        base = baseline[i]
        active = dict(struct.iter_unpack('<If', bank_bytes[12+i*1156:12+i*1156+1152]))
        route = int(active[48])
        assert route in (0,1,2,3)
        oracle, events = out/'original.f32', out/'events.i32'
        if i in focus:
            oracle, events = fixture/str(i)/'original.f32', fixture/str(i)/'events.i32'
            assert sha(events) == focus[i]['event_sha256']
        else:
            # The existing generated oracle includes the sequence observer, which
            # is restricted to sequencer rows. Use the ordinary oracle for guards.
            normal = targets.parent/'original'
            subprocess.run([normal, assets/'public-morphs.f32', assets/'compact.bin',
                            bank, str(i), oracle], check=True)
        assert sha(oracle) == base['original_sha256']
        original = np.fromfile(oracle, dtype='<f4').reshape(-1,2)
        row = {'index': i, 'guard': i in guards, 'route': route, 'arms': {}}
        for mode in (0,2) if i in focus else (0,):
            for candidate in (False, True):
                pcm, panel = out/'pcm.f32', out/'panel.u8'
                enabled = route if candidate and route in (1,2) else 0
                env = {**os.environ, 'S1_PROBE_ROUTE': str(enabled)}
                subprocess.run([out/'shipping', targets, str(i), pcm, panel, str(mode), events], check=True, env=env)
                assert list(panel.read_bytes()) == base['panel']
                if not candidate:
                    assert sha(pcm) == (base['stock_sha256'] if mode == 0 else focus[i]['modes']['oracle_events']['sha256'])
                test = np.fromfile(pcm, dtype='<f4').reshape(-1,2)
                scores = [scorer.score(original[:,c], test[:,c]) for c in (0,1)]
                row['arms'][f'{mode}:{int(candidate)}'] = {'sha256': sha(pcm), 'scores': scores,
                                                          'passed': all(s['pass'] for s in scores)}
        result.append(row)
    (out/'result.json').write_text(json.dumps({'source': SOURCE, 'bank_sha256': sha(bank), 'rows': result}, indent=2)+'\n')


if __name__ == '__main__':
    main()
