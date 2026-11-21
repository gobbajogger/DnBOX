#!/usr/bin/env python3
"""Native CP/MA association positive and compiled current-source runtime negative."""
import os
from pathlib import Path
import signal
import subprocess

root = Path(__file__).resolve().parents[2]
out = Path(os.environ.get('KIT_SECTION_OUT', str(root / 'build/host/drum-kit-section')))
out.mkdir(parents=True, exist_ok=True)

def run(name, source=None):
    env = dict(os.environ, SMC_UI_TEST_FILE='tests/host/ui_drum_kit_section_test.c', SMC_UI_OUT=str(out / name))
    if source:
        env['X0X_HOST_DEFS'] = env.get('X0X_HOST_DEFS', '') + ' -DX0X_HOST_UI_SOURCE="' + str(source) + '"'
    proc = subprocess.Popen(['sh', 'tests/host/smc_ui_test.sh'], cwd=root, env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, start_new_session=True)
    try:
        output, _ = proc.communicate(timeout=120)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        output, _ = proc.communicate()
        (out / (name + '.log')).write_text(output)
        raise SystemExit(name + ': timed out; owned process group killed and reaped')
    (out / (name + '.log')).write_text(output)
    (out / (name + '.exit')).write_text(str(proc.returncode) + '\n')
    return proc.returncode, output

code, output = run('positive')
if code or 'PASS CP/MA real framebuffer' not in output:
    raise SystemExit(output[-4000:])

include = (root / 'firmware/src/app/ui_drum_voice.inc').read_text()
anchor = 'cv_text(11,28-off,&FONT_T,page==0?"SOUND":"KIT",ink);'
if include.count(anchor) != 1:
    raise SystemExit('current-source association mutant anchor absent or ambiguous')
mutant = out / 'wrong-association-source'
mutant.mkdir(exist_ok=True)
(mutant / 'ui.c').write_bytes((root / 'firmware/src/app/ui.c').read_bytes())
(mutant / 'ui_drum_voice.inc').write_text(include.replace(anchor, 'cv_text(11,28-off,&FONT_T,"SOUND",ink);'))
code, output = run('wrong-association', mutant / 'ui.c')
if not code or 'KIT_SECTION_PAGE_ASSOCIATION_WRONG' not in output or 'FAIL ' not in output:
    raise SystemExit('compiled runtime association mutant was not rejected as intended:\n' + output[-4000:])
print('PASS native CP/MA SOUND/KIT association and current-source compiled runtime negative')
