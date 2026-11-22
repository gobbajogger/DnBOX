#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compare post-filter voice sums before effects with the same sequencer events."""
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


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inject(path, dest, changes):
    source = path.read_text()
    for old, new in changes:
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    dest.write_text(source)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sequence_replay', type=Path)
    parser.add_argument('panel_targets', type=Path)
    parser.add_argument('factory_build', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    fixture, targets, build, out = [
        p.resolve() for p in (args.sequence_replay, args.panel_targets, args.factory_build, args.output)
    ]
    out.mkdir()
    spec = importlib.util.spec_from_file_location('scorer', HERE.parent/'replay.py')
    scorer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(scorer)
    assert (fixture/'focused-results.json').read_bytes() == (HERE/'focused-results.json').read_bytes()
    assert sha(targets) == 'd72756ce2b5898b275e65f5b301527f2e687eb7df6c1c0ea0a8c3d5967237833'
    commands = json.loads((fixture/'commands.json').read_text())
    inject(fixture/'original.cpp', out/'original.cpp', [
        ('    unsigned frame{};', '    unsigned frame{};\n    FILE *dry{};'),
        ('    if (out.mono !=', '    CHECK(std::fwrite(out.left + frame, sizeof(float), 1, out.dry) == 1);\n    if (out.mono !='),
        ('CHECK(argc == 7);', 'CHECK(argc == 8);'),
        ('    Output out;', '    Output out;\n    out.dry = std::fopen(argv[7], "wb");\n    CHECK(out.dry);'),
        ('std::fclose(events) == 0);', 'std::fclose(events) == 0 && std::fclose(out.dry) == 0);'),
    ])
    inject(fixture/'shipping.c', out/'shipping.c', [
        ('static void key(', 'FILE *s1_voice_tap;\nstatic void key('),
        ('assert(argc == 7);', 'assert(argc == 8);\n    s1_voice_tap = fopen(argv[7], "wb");\n    assert(s1_voice_tap);'),
        ('    if (events) assert(fclose(events) == 0);', '    if (events) assert(fclose(events) == 0);\n    assert(fclose(s1_voice_tap) == 0);'),
    ])
    site = '    if (left != right) memcpy(right, left, frames * sizeof *left);'
    inject(ROOT/VOICE, out/'voice.c', [
        ('#include <string.h>', '#include <string.h>\n#include <stdio.h>\n#include <assert.h>\nextern FILE *s1_voice_tap;'),
        (site, '    assert(fwrite(left, sizeof(float), frames, s1_voice_tap) == frames);\n'+site),
    ])
    for key in ('compile_original', 'link_original', 'compile_candidate'):
        cmd = commands[key]
        cmd = [str(out/Path(x).name) if Path(x).parent == fixture else x for x in cmd]
        if VOICE in cmd:
            cmd[cmd.index(VOICE)] = str(out/'voice.c')
        subprocess.run(cmd, check=True, cwd=ROOT)
    assets = build/'factory-inputs'
    bank = build/'commands/preset-commands.bin'
    rows = []
    for case in json.loads((HERE/'focused-results.json').read_text())['rows']:
        i = case['index']
        original, firmware = out/'original.f32', out/'firmware.f32'
        original_dry, firmware_dry = out/'original-dry.f32', out/'firmware-dry.f32'
        events, panel = out/'events.i32', out/'panel.u8'
        subprocess.run([out/'original', assets/'public-morphs.f32', assets/'compact.bin',
                        bank, str(i), original, events, original_dry], check=True)
        subprocess.run([out/'shipping', targets, str(i), firmware, panel, '2', events, firmware_dry], check=True)
        assert sha(events) == case['event_sha256']
        assert sha(original) == case['original_sha256']
        assert sha(firmware) == case['modes']['oracle_events']['sha256']
        assert panel.read_bytes() == (fixture/str(i)/'direct.u8').read_bytes()
        a, b = (np.fromfile(p, dtype='<f4') for p in (original_dry, firmware_dry))
        assert len(a) == len(b) == 132300 and np.isfinite(a).all() and np.isfinite(b).all()
        rows.append({'index': i, 'events_sha256': sha(events), 'original_dry_sha256': sha(original_dry),
                     'firmware_dry_sha256': sha(firmware_dry), 'dry_score': scorer.score(a,b),
                     'untouched_final_channels': case['modes']['oracle_events']['channels']})
    (out/'result.json').write_text(json.dumps({'source': 'f6c2f17087dea9de52dc77f88dd271aab6692ef1',
                                             'bank_sha256': sha(bank), 'rows': rows}, indent=2)+'\n')


if __name__ == '__main__':
    main()
