#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Probe individual oscillator contributions in the matched original renderer."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

import numpy as np

HERE = Path(__file__).resolve().parent
OSC1 = '    sp_oscmorph2d_compute(kernel->spp(), oscmorph1, nil, &oscmorph1_out);'
OSC2 = '    oscmorph2_out *= getParam(morph2Volume);'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def build_probe(out, mode, original_compile, original_link):
    compile = original_compile.copy()
    source = Path(compile[compile.index('-c')+1]).read_text()
    if mode == 'osc1_ramp':
        assert source.count(OSC1) == 1
        source = source.replace(OSC1, '''
    const float phase = (float)oscmorph1->lphs * (1.0f / (float)(SP_FT_PHMASK + 1u));
''' + OSC1 + '''
    float raw = 2.0f * phase - 1.0f;
    if (phase >= 4095.f / 4096.f)
        raw -= 2.f * (4096.f * phase - 4095.f);
    oscmorph1_out = raw * oscmorph1->amp;''')
    if mode == 'osc2_silent':
        assert source.count(OSC2) == 1
        source = source.replace(OSC2, '    oscmorph2_out = 0.f;')
    if mode == 'fm_silent':
        site = '    fmOsc_out *= getParam(fmVolume);'
        assert source.count(site) == 1
        source = source.replace(site, '    fmOsc_out = 0.f;')
    if mode == 'cutoff_lfo_off':
        start = source.index('    if (getParam(cutoffLFO) == 1.f)')
        end = source.index('    // filter frequency env lfo crossfade', start)
        source = source[:start] + source[end:]
    if mode == 'env_lfo_off':
        start = source.index('    if (getParam(filterEnvLFO) == 1.f)')
        end = source.index('    // filter frequency mixer', start)
        source = source[:start] + source[end:]
    cpp = out/f'{mode}.cpp'
    obj = out/f'{mode}.o'
    cpp.write_text(source)
    obj_index = compile.index('-o')+1
    old_obj = compile[obj_index]
    compile[compile.index('-c')+1] = str(cpp)
    compile[obj_index] = str(obj)
    compile[compile.index('-MF')+1] = str(out/f'{mode}.d')
    subprocess.run(compile, check=True)
    link = original_link.copy()
    link[link.index(old_obj)] = str(obj)
    link[link.index('-o')+1] = str(out/mode)
    subprocess.run(link, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sequence_replay', type=Path)
    parser.add_argument('factory_build', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    fixture, build, out = [p.resolve() for p in (args.sequence_replay, args.factory_build, args.output)]
    out.mkdir()
    spec = importlib.util.spec_from_file_location('scorer', HERE.parent/'replay.py')
    scorer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(scorer)
    assert (fixture/'focused-results.json').read_bytes() == (HERE/'focused-results.json').read_bytes()
    link = json.loads((fixture/'commands.json').read_text())['link_original']
    compile = next(c for c in json.loads((build/'commands.json').read_text())
                   if any(s.endswith('/portable_voice.cpp') for s in c))
    modes = ('baseline', 'osc1_ramp', 'osc2_silent', 'fm_silent', 'cutoff_lfo_off', 'env_lfo_off')
    for mode in modes:
        build_probe(out, mode, compile, link)
    assets = build/'factory-inputs'
    bank = build/'commands/preset-commands.bin'
    rows = []
    for case in json.loads((HERE/'focused-results.json').read_text())['rows']:
        i = case['index']
        inputs = fixture/str(i)
        assert sha(inputs/'original.f32') == case['original_sha256']
        assert sha(inputs/'oracle_events.f32') == case['modes']['oracle_events']['sha256']
        original = np.fromfile(inputs/'original.f32', dtype='<f4').reshape(-1, 2)
        firmware = np.fromfile(inputs/'oracle_events.f32', dtype='<f4').reshape(-1, 2)
        row = {'index': i, 'events_sha256': case['event_sha256'], 'arms': {}}
        for mode in modes:
            pcm, events = out/'pcm.f32', out/'events.i32'
            subprocess.run([out/mode, assets/'public-morphs.f32', assets/'compact.bin',
                            bank, str(i), pcm, events], check=True)
            assert sha(events) == case['event_sha256']
            if mode == 'baseline':
                assert sha(pcm) == case['original_sha256']
            probe = np.fromfile(pcm, dtype='<f4').reshape(-1, 2)
            assert np.isfinite(probe).all()
            row['arms'][mode] = {'sha256': sha(pcm),
                                 'original_vs_probe': [scorer.score(original[:,c], probe[:,c]) for c in (0,1)],
                                 'probe_vs_firmware': [scorer.score(probe[:,c], firmware[:,c]) for c in (0,1)]}
        rows.append(row)
    (out/'result.json').write_text(json.dumps({'source': 'f6c2f17087dea9de52dc77f88dd271aab6692ef1',
                                             'bank_sha256': sha(bank), 'rows': rows}, indent=2)+'\n')


if __name__ == '__main__':
    main()
