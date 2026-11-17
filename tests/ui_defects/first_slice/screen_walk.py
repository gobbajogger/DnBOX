#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Render-presence gate. Never creates references or certifies visual conformance."""
import json, os, re, subprocess
from pathlib import Path
from PIL import Image
REQUIRED_FAMILIES=('ordinary','global','list','engine','help','modal')
# Deliberately frozen separately from runtime enumeration. A successful short walk
# must not certify its own coverage by reporting a smaller completion count.
def expected_inventory():
    manifest=json.loads(Path(__file__).with_name('screen_inventory.json').read_text())
    rows=manifest['screens']
    expected={row['id']:row['family'] for row in rows}
    if manifest.get('schema')!=1 or not expected or len(expected)!=len(rows):
        raise ValueError('invalid required screen inventory')
    return expected

def require_drum_source(root):
    source=root/'firmware/src/app/ui_drum_voice.inc'
    if not source.is_file():
        raise FileNotFoundError('required drum UI source missing: '+str(source))

def validate(log, root, renderer_exit):
    if renderer_exit:
        raise ValueError(f'host screen walker failed: exit {renderer_exit}')
    screens=[]; complete=None; seen=set(); families=set()
    for line in log.splitlines():
        if not line.startswith('WALK '):continue
        try: event=json.loads(line[5:])
        except ValueError as e:raise ValueError('malformed WALK record') from e
        if not isinstance(event,dict) or complete is not None:raise ValueError('invalid or trailing WALK record')
        if event.get('kind')=='complete':complete=event.get('screens');continue
        name=event.get('id'); family=event.get('family')
        if event.get('kind')!='screen' or not isinstance(name,str) or not re.fullmatch(r'[A-Za-z0-9_-]+',name) or name in seen or family not in REQUIRED_FAMILIES:
            raise ValueError('invalid or duplicate screen record')
        path=root/(name+'.png')
        if path.is_symlink() or not path.is_file():raise ValueError('missing or linked frame: '+name)
        try:
            with Image.open(path) as image:
                image.load()
                if image.format!='PNG' or image.size!=(240,240):raise ValueError('invalid frame dimensions/format: '+name)
                if all(lo==hi for lo,hi in image.convert('RGB').getextrema()):raise ValueError('blank frame: '+name)
        except OSError as e:raise ValueError('unreadable frame: '+name) from e
        seen.add(name);families.add(family);screens.append(event)
    if not screens or type(complete) is not int or complete!=len(screens):raise ValueError('empty or incomplete screen inventory')
    if families!=set(REQUIRED_FAMILIES):raise ValueError('missing screen family')
    expected=expected_inventory()
    observed={row['id']:row['family'] for row in screens}
    if observed!=expected:
        missing=sorted(expected.keys()-observed.keys())
        extra=sorted(observed.keys()-expected.keys())
        changed=sorted(k for k in expected.keys() & observed.keys() if expected[k]!=observed[k])
        raise ValueError(f'screen coverage mismatch: missing={missing[:10]} extra={extra[:10]} changed={changed[:10]}')
    return {'screens':len(screens),'families':sorted(families),'inventory':screens,'visualApproval':False}

def run(work, out):
    out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy();env.update(SMC_UI_OUT=str(out),SMC_UI_TEST_FILE='tests/host/ui_screen_walk.c')
    logpath=out/'renderer.log'
    with logpath.open('w') as log:
        proc=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=900)
    result=validate(logpath.read_text(),out,proc.returncode)
    (out/'inventory.json').write_text(json.dumps(result,indent=2)+'\n')
    print(f"ui-screen-walk: rendered={result['screens']} (render presence only, not visual approval)")
    return result
