#!/usr/bin/env python3
"""Replay the measured field check from pinned bytes, including its three negatives."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
MANIFEST = '6f7f1eb573834d45ffaf1758a26b73b0f5e906132fd852458ddf96af36ccab3a'


def checked(path, digest):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != digest:
        raise ValueError(f'Pinned input changed: {path}')
    return data


def safe_relative(name):
    path = Path(name)
    if path.is_absolute() or '..' in path.parts:
        raise ValueError(f'Invalid manifest path: {name}')
    return path


def fresh_output(path):
    path = path if path.is_absolute() else Path.cwd() / path
    if any(p.is_symlink() for p in (path, *path.parents)) or path.exists():
        raise ValueError(f'Output already exists or traverses a symlink: {path}')
    return path.resolve()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    output = fresh_output(args.out)
    manifest = json.loads(checked(HERE / 'IMPORT-MANIFEST.json', MANIFEST))
    overlay = {item['path']: checked(HERE / safe_relative(item['path']), item['sha256'])
               for item in manifest['files'] if not item['path'].startswith('evidence/')}
    core = {name: checked(args.repo / safe_relative(name), digest)
            for name, digest in json.loads(overlay['INPUTS.json'])['files'].items()}
    source = overlay['sj6_preset_fields.c']
    for anchor in (b'status = dnbx_set_layer(part, 0, &layer);', b'has_attack = 1;'):
        if source.count(anchor) != 1:
            raise ValueError('Mutation anchor is absent or ambiguous')
    blocked = ('CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH',
               'COMPILER_PATH', 'GCC_EXEC_PREFIX', 'LD_PRELOAD', 'LD_LIBRARY_PATH')
    if any(os.environ.get(key) for key in blocked):
        raise ValueError('Unpinned compiler include, tool or library override')
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        for name, data in {**core, **{f'overlay/{k}': v for k, v in overlay.items()}}.items():
            target = root / safe_relative(name)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        # -I ignores PYTHONOPTIMIZE and removes user-site/path overrides. The
        # historical measured runner keeps its original bytes and unique-anchor
        # assertion; the supported entry checks those anchors explicitly above.
        return subprocess.run([sys.executable, '-I', str(root/'overlay/verify.py'),
                               '--repo', str(root), '--out', str(output), '--cc', args.cc],
                              check=False).returncode


if __name__ == '__main__':
    raise SystemExit(main())
