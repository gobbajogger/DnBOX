#!/usr/bin/env python3
"""Bounded active-runner sanitizer control scan, not runtime leak qualification."""
import os, pathlib, re, sys

DISABLE = re.compile(r'(?:detect_leaks|leak_check_at_exit)\s*=\s*(?:0|false)\b|__lsan_(?:disable|ignore_object)\s*\(|-fno-sanitize=(?:[^\s,]+,)*leak\b', re.I)
SUPPRESS = re.compile(r'(?:ASAN_OPTIONS|LSAN_OPTIONS)[^\n]*(?:suppressions\s*=|exitcode\s*=\s*0\b)', re.I)
def inspect(root, env):
    errors = []
    for name in ('ASAN_OPTIONS', 'LSAN_OPTIONS'):
        value = env.get(name, '')
        if DISABLE.search(value) or re.search(r'(?:^|:)\s*(?:suppressions\s*=|exitcode\s*=\s*0\b)', value):
            errors.append('environment: ' + name)
    paths = [root / 'tests/run_tests.sh']
    if not paths[0].is_file():
        errors.append('missing required runner: tests/run_tests.sh')
    for folder in ('tests/host', 'tools/mboot'):
        if not (root / folder).is_dir():
            errors.append('missing required subtree: ' + folder)
        paths.extend(p for p in (root / folder).rglob('*') if p.suffix in ('.sh', '.py', '.c', '.h') and p.is_file())
    count = 0
    for path in paths:
        if not path.is_file():
            continue
        count += 1
        source = path.read_text()
        if DISABLE.search(source) or SUPPRESS.search(source):
            errors.append(str(path.relative_to(root)))
    if not count:
        errors.append('missing active-runner inventory')
    return count, errors
def main():
    root = pathlib.Path(sys.argv[1]) if len(sys.argv) == 2 else pathlib.Path(__file__).resolve().parents[1]
    count, errors = inspect(root, os.environ)
    for name in errors:
        print('FAIL sanitizer control: ' + name)
    print(f'active-runner policy: {count} files; historical/external input call-graph and runtime checks remain separate')
    return bool(errors)
if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, UnicodeError):
        print('FAIL sanitizer policy inventory unreadable', file=sys.stderr)
        sys.exit(1)
