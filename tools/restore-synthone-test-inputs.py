#!/usr/bin/env python3
"""Restore byte-pinned source/test inputs, never execute archived commands here."""
import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path
import tarfile


def digest(data):
    return hashlib.sha256(data).hexdigest()


def restore(bundle, output):
    if output.exists():
        raise ValueError('output must not exist')
    manifest = json.loads((bundle / 'MANIFEST.json').read_text())
    chunks = []
    for row in manifest['parts']:
        if Path(row['file']).name != row['file']:
            raise ValueError('invalid part path')
        b = (bundle / row['file']).read_bytes()
        if len(b) != row['bytes'] or digest(b) != row['sha256']:
            raise ValueError('part hash/length mismatch')
        chunks.append(b)
    archive = b''.join(chunks)
    if len(archive) != manifest['archive_bytes'] or digest(archive) != manifest['archive_sha256']:
        raise ValueError('archive hash/length mismatch')
    expected = {r['path']: r for r in manifest['files']}
    if len(expected) != len(manifest['files']):
        raise ValueError('duplicate manifest member')
    data = {}
    with tarfile.open(fileobj=io.BytesIO(gzip.decompress(archive)), mode='r:') as tar:
        for member in tar.getmembers():
            path = Path(member.name)
            if not member.isfile() or path.is_absolute() or '..' in path.parts or member.name in data or member.name not in expected:
                raise ValueError('unlisted/unsafe archive member')
            b = tar.extractfile(member).read()
            row = expected[member.name]
            if len(b) != row['bytes'] or digest(b) != row['sha256']:
                raise ValueError('member hash/length mismatch')
            data[member.name] = b
    if set(data) != set(expected):
        raise ValueError('missing archive member')
    # All parts/members verified before writing anything. Relocation changes only
    # the documented placeholder; the strict runner validates every actual path.
    commands = json.loads(data['commands.template.json'])
    relocated = [[value.replace('@SOURCE_BUILD@', str(output.resolve())) for value in command] for command in commands]
    for name, b in data.items():
        path = output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b)
    (output / 'commands.json').write_text(json.dumps(relocated, indent=2) + '\n')
    (output / 'RESTORE-RECEIPT.json').write_text(json.dumps({
        'archive_sha256': manifest['archive_sha256'], 'files_verified': len(data),
        'template_sha256': digest(data['commands.template.json']),
        'relocated_commands_sha256': digest((output / 'commands.json').read_bytes())}, indent=2) + '\n')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bundle', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    restore(a.bundle.resolve(), a.output.resolve())
