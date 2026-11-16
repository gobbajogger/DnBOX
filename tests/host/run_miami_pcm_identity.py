#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Output-only Miami identity against a local Git commit.

Builds tests/host/miami_cpu_test.c with -DMIAMI_TRACE_PCM_ONLY against the
current tree and against `git archive <ref>`, runs proof, idle-proof and
idle-range-proof, and requires byte-equal PCM, dry and scratch traces. Use it
when a change adds derived cache fields, so full state traces differ by
design. No network, checkout, staging or push occurs."""
import argparse, importlib.util, io, os, subprocess, sys, tarfile, types
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('mct', ROOT/'tests/host/miami_cpu_test.py')
mct = importlib.util.module_from_spec(spec); spec.loader.exec_module(mct)
p = argparse.ArgumentParser(); p.add_argument('ref'); p.add_argument('--out', default='build/host/miami-pcm-identity')
a = p.parse_args()
out = (ROOT/a.out).resolve(); subprocess.run(['rm', '-rf', str(out)], check=True); out.mkdir(parents=True)
ref = out/'ref-source'; ref.mkdir()
data = subprocess.check_output(['git', 'archive', '--format=tar', a.ref, 'experiments', 'firmware/src', 'tests/host', 'tools'], cwd=ROOT)
with tarfile.open(fileobj=io.BytesIO(data)) as t:
    t.extractall(ref, filter='data')
os.environ['CC'] = os.environ.get('CC', 'cc')+' -DMIAMI_TRACE_PCM_ONLY'
args = types.SimpleNamespace(native_fma=False, sanitize=False, profile=False, idle_proxy=False)
for side, src in (('ref', ref), ('cand', ROOT)):
    mct.build(src, out/side, args)
    for mode in ('proof', 'idle-proof', 'idle-range-proof'):
        subprocess.run([str(out/side/'runner'), mode, str(out/side/(mode+'.bin'))], check=True, stdout=subprocess.DEVNULL)
bad = 0
for mode in ('proof', 'idle-proof', 'idle-range-proof'):
    r, c = (out/'ref'/(mode+'.bin')).read_bytes(), (out/'cand'/(mode+'.bin')).read_bytes()
    ok = r == c and len(r) > 0
    bad |= not ok
    print(('PASS' if ok else 'FAIL'), 'miami-pcm-identity', mode, len(c), 'bytes')
sys.exit(1 if bad else 0)
