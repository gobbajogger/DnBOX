#!/usr/bin/env python3
"""Run this field adapter against the pinned independent DnBOX candidate."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import time

HERE = Path(__file__).resolve().parent
CORE_DIR = Path('experiments/sound-hardware-v1/hardware_candidate/src')
FIXTURE_DIR = Path('experiments/sound-hardware-v1/mapping/results/implementer_brief')
FLAGS = ['-std=c11', '-O2', '-DX0X_HOST', '-ffp-contract=off', '-fno-fast-math',
         '-Wall', '-Wextra', '-Werror']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    repo = args.repo.resolve()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    pins = json.loads((HERE/'INPUTS.json').read_text())
    for rel, expected in pins['files'].items():
        if sha(repo/rel) != expected:
            raise SystemExit(f'frozen input changed: {rel}')
    source = (HERE/'sj6_preset_fields.c').read_text()
    wrong_layer = 'status = dnbx_set_layer(part, 0, &layer);'
    linear_law = 'has_attack = 1;'
    assert source.count(wrong_layer) == source.count(linear_law) == 1
    versions = [
        ('red_raw_only', HERE/'tests/raw_storage_baseline.c', None, 1, 'stored but layer 1 attack'),
        ('green', HERE/'sj6_preset_fields.c', None, 0, 'PASS raw status'),
        ('wrong_layer', None, source.replace(wrong_layer, wrong_layer.replace('part, 0','part, 1')), 1, 'stored but layer 1 attack'),
        ('linear_attack', None, source.replace(linear_law, 'layer.amp.attack_s = .002f + 15.998f * field.value;\n            '+linear_law), 1, 'stored but layer 1 attack'),
    ]
    compiler = subprocess.run([args.cc,'--version'],capture_output=True,text=True,check=True).stdout.splitlines()[0]
    receipt = {'source_base':pins['source_base'], 'input_files':pins['files'], 'cc':compiler,
               'input_source_boundary':'Only frozen independently written DnBOX candidate and published source-free behavior fixtures',
               'adapter_files':{p.relative_to(HERE).as_posix():sha(p) for p in [HERE/'sj6_preset_fields.c',HERE/'sj6_preset_fields.h',HERE/'tests/test_id36.c',HERE/'tests/raw_storage_baseline.c',HERE/'verify.py']},
               'cases':[]}
    started = time.monotonic()
    with tempfile.TemporaryDirectory() as tmp:
        for name, src, mutated, expected, observed_text in versions:
            if mutated is not None:
                src = Path(tmp)/(name+'.c')
                src.write_text(mutated)
            binary = Path(tmp)/name
            cmd = [args.cc,*FLAGS,'-I'+str(repo/CORE_DIR),'-I'+str(HERE),str(HERE/'tests/test_id36.c'),str(src),str(repo/CORE_DIR/'dnbx_core.c'),'-lm','-o',str(binary)]
            build = subprocess.run(cmd,capture_output=True,text=True)
            (out/(name+'.build.log')).write_text(build.stdout+build.stderr)
            if build.returncode:
                raise SystemExit(f'{name} compile exit {build.returncode}')
            ran = subprocess.run([str(binary)],capture_output=True,text=True)
            (out/(name+'.stdout')).write_text(ran.stdout)
            (out/(name+'.stderr')).write_text(ran.stderr)
            passed = ran.returncode == expected and observed_text in (ran.stdout+ran.stderr)
            receipt['cases'].append({'name':name,'adapter_sha256':sha(src),'compile_argv':cmd,
                                     'compile_exit':build.returncode,'run_argv':[str(binary)],
                                     'actual_exit':ran.returncode,'expected_exit':expected,'expected_text_seen':passed})
            print(f'{name}: compile={build.returncode} run={ran.returncode} expected={expected} verdict={"PASS" if passed else "FAIL"}')
    receipt['passed'] = all(c['expected_text_seen'] for c in receipt['cases'])
    receipt['seconds'] = time.monotonic()-started
    receipt['scope'] = 'Five link-OFF layer-1 field settings, 10240 stereo frames at 48000 Hz and tested admission cases. No full original preset playback or target qualification.'
    (out/'RESULT.json').write_text(json.dumps(receipt,indent=2)+'\n')
    return 0 if receipt['passed'] else 1

if __name__ == '__main__':
    raise SystemExit(main())
