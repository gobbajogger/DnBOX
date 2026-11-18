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
def sanitizer_environment():
    env = os.environ.copy()
    # Qualification must not inherit suppressions or disabling scanner flags.
    env['ASAN_OPTIONS'] = 'detect_leaks=1:exitcode=23:halt_on_error=1'
    env['LSAN_OPTIONS'] = 'detect_leaks=1:exitcode=23'
    env['UBSAN_OPTIONS'] = 'halt_on_error=1'
    return env


def run_sanitized(exe):
    subprocess.run([str(exe)], check=True, env=sanitizer_environment())


def require_leak_control(result):
    report = (result.stdout or '') + (result.stderr or '')
    if (result.returncode != 23 or 'LeakSanitizer: detected memory leaks' not in report
            or not re.search(r'123 byte\(s\) leaked in 1 allocation\(s\)', report)):
        raise RuntimeError('LeakSanitizer control failed: expected exit 23 and a real 123-byte leak report; '
                           + f'got exit {result.returncode}\n' + report)


def main():
    DATA = ROOT / 'experiments/original_synth'
    header = DATA / 'program_values_compact.h'
    before = header.read_bytes()
    compiler = [*shlex.split(os.environ.get('CC', 'cc'))]
    sanitizer_flags = ['-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
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
        # Prove leak detection works before trusting the leak-free program run.
        leak_source = tmp/'leak-control.c'
        leak_source.write_text('#include <stdlib.h>\n__attribute__((noinline)) void leak(void) {\n'
                               '  char *volatile p = malloc(123); if (!p) abort(); p[0] = 1;\n'
                               '}\nint main(void) { leak(); return 0; }\n')
        leak_exe = tmp/'leak-control'
        subprocess.run([*compiler, *sanitizer_flags, str(leak_source), '-o', str(leak_exe)], check=True)
        control = subprocess.run([str(leak_exe)], capture_output=True, text=True, env=sanitizer_environment())
        require_leak_control(control)
        print('PASS genuine 123-byte LeakSanitizer negative control rejected')
        for flags in (['-O2'], sanitizer_flags):
            exe = tmp/'test'
            command = [*shlex.split(os.environ.get('CC', 'cc')), '-std=c99', '-Wall', '-Wextra',
                       '-Werror', '-pedantic', *flags, '-I'+str(tmp), '-I'+str(DATA),
                       str(ROOT/'tests/host/program_values_compact_test.c'), '-o', str(exe)]
            subprocess.run(command, check=True)
            if flags == sanitizer_flags:
                run_sanitized(exe)
            else:
                subprocess.run([str(exe)], check=True)
        # Exercise the oracle, not only a successful compile of its expected values.
        changed = re.sub(r'0x([0-9a-f]{8})u',
                         lambda m: f'0x{int(m[1], 16) ^ 1:08x}u', before.decode(), count=1)
        (tmp/header.name).write_text(changed)
        subprocess.run([*shlex.split(os.environ.get('CC', 'cc')), '-std=c99', '-O2',
                        '-I'+str(tmp), '-I'+str(DATA), str(ROOT/'tests/host/program_values_compact_test.c'),
                        '-o', str(tmp/'mutant')], check=True)
        if subprocess.run([str(tmp/'mutant')], capture_output=True).returncode == 0:
            raise RuntimeError('metadata corruption negative control escaped')
        print('PASS metadata corruption negative control rejected')

if __name__ == "__main__":
    main()
