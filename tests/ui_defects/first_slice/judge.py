#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Gate entry: render the checked-out firmware screens on the host and judge them.

Exports the committed HEAD tree into a fresh temporary directory, applies the
test-only instrumentation, runs the real host renderer once, and runs the
checks 1-4 detectors plus the glyph/value collision check on every screen.
Exit 0 only when every screen renders and no check fails or is NOT_RUN.
No receipts are read; nothing outside the output directory is written.
"""
import argparse, json, subprocess, sys, tempfile, os
from pathlib import Path
from instrument import instrument, HERE
from detectors import inspect_screen
from run import parse_trace, write

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--rev', default='HEAD')
    a = p.parse_args()
    out = a.output.resolve(); out.mkdir(parents=True, exist_ok=False)
    repo = Path(subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], cwd=HERE, text=True).strip())
    with tempfile.TemporaryDirectory(prefix='dnbox-ui-judge-') as tmp:
        work = Path(tmp) / 'src'; work.mkdir()
        archive = subprocess.run(['git', 'archive', a.rev], cwd=repo, check=True, stdout=subprocess.PIPE).stdout
        subprocess.run(['tar', '-x', '-C', str(work)], input=archive, check=True)
        if not (work / 'firmware/src/app/ui_drum_voice.inc').exists():
            # The first-slice detectors target the R22 drum-voice screen. Until the
            # fwui lane lands it, there is no such screen in this tree to judge.
            print('ui-defects: no R22 drum-voice screen (firmware/src/app/ui_drum_voice.inc) in this tree; '
                  'first-slice render not applicable')
            write(out / 'judgement.json', {'applicable': False})
            return 0
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
