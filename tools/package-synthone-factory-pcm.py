#!/usr/bin/env python3
"""Verify every expected PCM byte, then make one deterministic paired-output archive.
Original and compact names remain recoverable; identical pairs use tar hardlinks.
Uncompressed USTAR avoids cross-environment compressor-version byte differences.
Does not upload, delete originals, edit Git refs or mark any sound gate PASS.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import tarfile


def validate_pairs(names):
    compact = {name for name in names if re.fullmatch(r'preset-\d+-compact\.f32le\.pcm', name)}
    original = {name for name in names if re.fullmatch(r'preset-\d+-original\.f32le\.pcm', name)}
    if len(compact) != 694 or len(original) != 694 or compact | original != set(names) or original != {name.replace('-compact.', '-original.') for name in compact}:
        raise ValueError('missing/unpaired original/compact PCM files')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--pcm', type=Path, required=True)
    p.add_argument('--expected-manifest', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('archive output must not exist')
    manifest_bytes = args.expected_manifest.read_bytes()
    manifest = json.loads(manifest_bytes)
    expected = {row['file']: row for row in manifest['files']}
    if len(expected) != 1388 or len(expected) != len(manifest['files']):
        raise ValueError('expected exactly1388 unique original/compact files')
    validate_pairs(expected)
    actual = {path.name for path in args.pcm.glob('*.pcm')}
    if actual != set(expected):
        raise ValueError('missing or extra PCM files')
    # Verify the whole collection before creating the archive, then recheck bytes
    # as they are consumed. A different-platform reconstruction must match exactly.
    def checked(name):
        if not re.fullmatch(r'preset-\d+-(original|compact)\.f32le\.pcm', name):
            raise ValueError('invalid PCM name')
        b = (args.pcm / name).read_bytes()
        row = expected[name]
        if len(b) != row['bytes'] or hashlib.sha256(b).hexdigest() != row['sha256']:
            raise ValueError('PCM output mismatch: ' + name)
        return b
    for name in sorted(expected):
        checked(name)
    with args.output.open('xb') as raw, tarfile.open(fileobj=raw, mode='w|', format=tarfile.USTAR_FORMAT) as tar:
        info = tarfile.TarInfo('PCM-MANIFEST.json');info.size=len(manifest_bytes);info.mode=0o644
        tar.addfile(info, io.BytesIO(manifest_bytes))
        for name in sorted(n for n in expected if '-compact.' in n):
            original = name.replace('-compact.', '-original.')
            b = checked(name)
            if checked(original) != b:
                raise ValueError('paired output differs: ' + original)
            info = tarfile.TarInfo('pcm/' + name);info.size=len(b);info.mode=0o644
            tar.addfile(info, io.BytesIO(b))
            link = tarfile.TarInfo('pcm/' + original);link.type=tarfile.LNKTYPE;link.linkname='pcm/' + name;link.mode=0o644
            tar.addfile(link)
    sha = hashlib.sha256()
    with args.output.open('rb') as f:
        for chunk in iter(lambda:f.read(1024*1024), b''):
            sha.update(chunk)
    print(json.dumps({'archive_bytes': args.output.stat().st_size, 'lfs_oid_sha256': sha.hexdigest(),
                      'manifest_sha256': hashlib.sha256(manifest_bytes).hexdigest(), 'recoverable_pcm_files':1388}))


if __name__ == '__main__':
    main()
