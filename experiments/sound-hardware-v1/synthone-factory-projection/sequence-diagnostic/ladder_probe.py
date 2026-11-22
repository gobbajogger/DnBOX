#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Measure the candidate low-pass ladder inside the original voice graph."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import numpy as np

HERE = Path(__file__).resolve().parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


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
    original_link = json.loads((fixture/'commands.json').read_text())['link_original']
    original_compile = json.loads((build/'commands.json').read_text())
    compile = next(c.copy() for c in original_compile if any(s.endswith('/moogladder.c') for s in c))
    compile[compile.index('-c')+1] = str(HERE/'ladder_probe.c')
    compile[compile.index('-o')+1] = str(out/'probe.o')
    compile[compile.index('-MF')+1] = str(out/'probe.d')
    subprocess.run(compile, check=True)
    original_link[original_link.index('-o')+1] = str(out/'original-with-ladder')
    original_link.extend([str(out/'probe.o'), '-Wl,--wrap=sp_moogladder_compute'])
    subprocess.run(original_link, check=True)
    assets = build/'factory-inputs'
    bank = build/'commands/preset-commands.bin'
    bank_bytes = bank.read_bytes()
    rows = []
    for case in json.loads((HERE/'focused-results.json').read_text())['rows']:
        i = case['index']
        inputs = fixture/str(i)
        pcm, events = out/'pcm.f32', out/'events.i32'
        subprocess.run([out/'original-with-ladder', assets/'public-morphs.f32', assets/'compact.bin',
                        bank, str(i), pcm, events], check=True)
        assert digest(inputs/'original.f32') == case['original_sha256']
        assert digest(inputs/'oracle_events.f32') == case['modes']['oracle_events']['sha256']
        assert digest(events) == case['event_sha256']
        original = np.fromfile(inputs/'original.f32', dtype='<f4').reshape(-1, 2)
        firmware = np.fromfile(inputs/'oracle_events.f32', dtype='<f4').reshape(-1, 2)
        probe = np.fromfile(pcm, dtype='<f4').reshape(-1, 2)
        assert np.isfinite(probe).all()
        controls = dict(struct.iter_unpack('<If', bank_bytes[12+i*1156:12+i*1156+1152]))
        rows.append({'index': i, 'selected_filter': int(controls[115]), 'events_sha256': digest(events),
                     'probe_sha256': digest(pcm), 'probe_equal_to_original': digest(pcm) == case['original_sha256'],
                     'original_vs_probe': [scorer.score(original[:,c], probe[:,c]) for c in (0,1)],
                     'probe_vs_firmware_matched_events': [scorer.score(probe[:,c], firmware[:,c]) for c in (0,1)]})
    (out/'result.json').write_text(json.dumps({'source': 'f6c2f17087dea9de52dc77f88dd271aab6692ef1',
                                             'bank_sha256': digest(bank), 'rows': rows}, indent=2)+'\n')


if __name__ == '__main__':
    main()
