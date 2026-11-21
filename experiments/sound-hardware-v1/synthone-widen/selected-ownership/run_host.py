#!/usr/bin/env python3
"""Build this diagnostic against a pristine or patched complete source tree."""
import argparse
import os
from pathlib import Path
import subprocess
import sys

here = Path(__file__).resolve().parent
parser = argparse.ArgumentParser()
parser.add_argument('source', type=Path)
parser.add_argument('out', type=Path)
parser.add_argument('--stock', action='store_true')
args = parser.parse_args()
source, out = args.source.resolve(), args.out.resolve()
out.mkdir(parents=True, exist_ok=True)
cc = out / 'compiler'
cc.write_text(f"""#!{sys.executable}
import os, sys
source = {str(here / 'fixture.c')!r}
args = [source if a == 'host/x0x_host.c' else a for a in sys.argv[1:]]
os.execvp('cc', ['cc', '-I.', *args])
""")
cc.chmod(0o755)
env = dict(os.environ, CC=str(cc), X0X_HOST_OUT=str(out / 'runner'))
if not args.stock:
    env['X0X_HOST_DEFS'] = env.get('X0X_HOST_DEFS', '') + ' -DX0X_WIDEN_PROBE'
with (out / 'build.log').open('w') as log:
    subprocess.run(['sh', 'host/build_host.sh'], cwd=source, env=env, stdout=log, stderr=log, check=True)
for mode in range(1 if args.stock else 2):
    with (out / f'run-{mode}.log').open('w') as log:
        subprocess.run([out/'runner', str(mode), out/f'pcm-{mode}.bin', out/f'proj-{mode}.bin'],
                       cwd=source, stdout=log, stderr=log, check=True)
