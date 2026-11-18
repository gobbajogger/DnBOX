#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Gate entry: render the checked-out firmware screens on the host and judge them.

Exports the committed HEAD tree into a fresh temporary directory, applies the
full-inventory geometry instrumentation and negative controls, then runs the
checks 1-4 detectors plus glyph/value collisions on the explicitly enumerated drum-page slice.
Exit 0 only when every screen renders and no check fails or is NOT_RUN.
No receipts are read; nothing outside the output directory is written.
"""
import argparse, json, subprocess, sys, tempfile, os
from pathlib import Path
from instrument import instrument, HERE
from detectors import inspect_screen
from run import parse_trace, write
from screen_walk import require_drum_source
from full_geometry import run as full_geometry

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--rev', default='HEAD')
    a = p.parse_args()
    out = a.output.resolve(); out.mkdir(parents=True, exist_ok=False)
    repo = Path(subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], cwd=HERE, text=True).strip())
    with tempfile.TemporaryDirectory(prefix='dnbox-ui-judge-', dir=out) as tmp:
        work = Path(tmp) / 'src'; work.mkdir()
        archive = subprocess.run(['git', 'archive', a.rev], cwd=repo, check=True, stdout=subprocess.PIPE).stdout
        subprocess.run(['tar', '-x', '-C', str(work)], input=archive, check=True)
        if not (work / 'firmware/src/app/ui_drum_voice.inc').exists():
            reason = 'required R22 renderer source missing: firmware/src/app/ui_drum_voice.inc'
            print('ui-defects: FAIL: ' + reason)
            write(out / 'judgement.json', {'status': 'FAIL', 'screens': 0, 'reason': reason})
            return 1
        try:
            full_geometry(work, out / 'geometry')
            require_drum_source(work)
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            write(out / 'judgement.json', {'status': 'FAIL', 'error': str(error)})
            print(f'ui-defects: FAIL: {error}')
            return 1
        instrument(work)
        env = os.environ.copy()
        env.update({'SMC_UI_OUT': str(out / 'host'), 'SMC_UI_TEST_FILE': 'tests/host/first_slice_capture.c'})
        logpath = out / 'renderer.log'
        with logpath.open('w') as log:
            proc = subprocess.run(['sh', 'tests/host/smc_ui_test.sh'], cwd=work, env=env,
                                  stdout=log, stderr=subprocess.STDOUT, timeout=900)
    if proc.returncode:
        print(f'ui-defects: host renderer failed (exit {proc.returncode}); see {logpath}')
        print(logpath.read_text()[-3000:])
        return 1
    screens, complete = parse_trace(logpath.read_text())
    inspected = [inspect_screen(s) for s in screens]
    bad = 0; summary = {}
    for i in range(1, 5):
        failures = [{'screen': s['id'], **f} for s in inspected for f in s['checks'][str(i)]['failures']]
        notrun = [s['id'] for s in inspected if s['checks'][str(i)]['status'] not in ('PASS', 'FAIL')]
        status = 'FAIL' if failures else ('NOT_RUN' if notrun else 'PASS')
        summary[str(i)] = {'status': status, 'failures': failures, 'notRun': notrun}
        if status != 'PASS': bad = 1
    overlaps = [{'screen': s['id'], **f} for s in inspected for f in s['overlaps']]
    if overlaps: bad = 1
    write(out / 'judgement.json', {'screens': len(screens), 'checks': summary, 'overlaps': overlaps})
    for i, c in summary.items():
        print(f"ui-defects check{i}: {c['status']} failures={len(c['failures'])} notrun={len(c['notRun'])}")
        for f in c['failures'][:10]: print('  ', json.dumps(f)[:300])
    print(f'ui-defects: screens={len(screens)} overlaps={len(overlaps)}')
    for f in overlaps[:10]: print('  ', json.dumps(f)[:300])
    return bad

if __name__ == '__main__':
    sys.exit(main())
