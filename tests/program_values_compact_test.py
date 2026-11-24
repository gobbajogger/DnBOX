#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reproduce immutable metadata, compare all raw words/names, reject corruption."""
from pathlib import Path
import os
import re
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    DATA = ROOT / 'experiments/original_synth'
    header = DATA / 'program_values_compact.h'
    before = header.read_bytes()
    compiler = [*shlex.split(os.environ.get('CC', 'cc'))]
    with tempfile.TemporaryDirectory(prefix='program-values-') as tmp:
        tmp = Path(tmp)
        generated = tmp / 'generated.h'
        stamp = header.stat().st_mtime_ns
        subprocess.run([sys.executable, str(ROOT/'tools/gen_program_values_compact.py'),
                        str(generated)], check=True)
        if generated.read_bytes() != before:
            raise RuntimeError('generated program values are stale')
        if header.read_bytes() != before or header.stat().st_mtime_ns != stamp:
            raise RuntimeError('generator modified the pinned source header')
        # LeakSanitizer qualification was FINAL-GOAL row 11 (Dot-only) and was
        # dropped by the user on 2026-11-24 00:02. Keep the program-value checks.
        exe = tmp/'test'
        command = [*compiler, '-std=c99', '-Wall', '-Wextra',
                   '-Werror', '-pedantic', '-O2', '-I'+str(tmp), '-I'+str(DATA),
                   str(ROOT/'tests/host/program_values_compact_test.c'), '-o', str(exe)]
        subprocess.run(command, check=True)
        subprocess.run([str(exe)], check=True)
        # Exercise the oracle, not only a successful compile of its expected values.
        changed = re.sub(r'0x([0-9a-f]{8})u',
                         lambda m: f'0x{int(m[1], 16) ^ 1:08x}u', before.decode(), count=1)
        (tmp/header.name).write_text(changed)
        subprocess.run([*compiler, '-std=c99', '-O2',
                        '-I'+str(tmp), '-I'+str(DATA), str(ROOT/'tests/host/program_values_compact_test.c'),
                        '-o', str(tmp/'mutant')], check=True)
        if subprocess.run([str(tmp/'mutant')], capture_output=True).returncode == 0:
            raise RuntimeError('metadata corruption negative control escaped')
        print('PASS metadata corruption negative control rejected')

if __name__ == "__main__":
    main()
