#!/usr/bin/env python3
"""Compare legacy-key records, not loader behavior or parameter migration."""
import argparse
import collections
import gzip
import hashlib
import json
import math
from pathlib import Path
import zipfile

KEYS = frozenset(('neonlimiterAmount', 'neoncompressorAmount', 'neontubeRageAmount'))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def records(banks):
    out = {}
    names = set()
    for bank in banks:
        name = bank['name']
        if not isinstance(name, str) or name in names:
            raise ValueError('invalid/duplicate bank identity')
        names.add(name)
        for preset in bank['presets']:
            uid = preset['uid']
            if not isinstance(uid, str) or not uid or uid in out:
                raise ValueError('invalid/duplicate preset identity')
            params = {}
            for item in preset['parameterValues']:
                key, value = item['identifier'], item['value']
                if not isinstance(key, str) or key in params:
                    raise ValueError('invalid/duplicate parameter identity')
                if isinstance(value, bool) or not isinstance(value, (float, int)) or not math.isfinite(value):
                    raise ValueError('invalid parameter value')
                params[key] = value
            subset = {key: params[key] for key in KEYS if key in params}
            if len(subset) != 1:
                raise ValueError('expected exactly one stored legacy amount key')
            out[uid] = (name, preset.get('version'), subset)
    return out


def compare(supplied, bundled):
    left, right = records(supplied), records(bundled)
    if {b['name'] for b in supplied} != {b['name'] for b in bundled}:
        raise ValueError('bank identity sets differ')
    if set(left) != set(right):
        raise ValueError('preset identity sets differ')
    mismatches = sum(left[k] != right[k] for k in left)
    return {'scope': __doc__, 'presets': len(left),
            'matching_legacy_records': len(left) - mismatches,
            'mismatched_legacy_records': mismatches,
            'stored_key_counts': dict(sorted(collections.Counter(
                next(iter(v[2])) for v in left.values()).items())),
            'migration_status': 'NOT_PROVEN',
            'loader_execution': 'NOT_RUN',
            'status': 'PASS' if not mismatches else 'FAIL'}


def run(root, manifest_path, archive_path, expected_archive_sha256):
    # Capture metadata once; every parsed member is hashed from the same bytes.
    mb = manifest_path.read_bytes()
    manifest = json.loads(mb)
    ab = archive_path.read_bytes()
    if sha(ab) != expected_archive_sha256:
        raise ValueError('archive SHA256 mismatch')
    import io
    supplied, bundled, inputs = [], [], []
    seen = set()
    for item in manifest['banks']:
        name = item['file']
        if Path(name).name != name or name in seen:
            raise ValueError('invalid/duplicate bank path')
        seen.add(name)
        data = (root / name).read_bytes()
        if len(data) != item['gzip_bytes'] or sha(data) != item['gzip_sha256']:
            raise ValueError('compressed input mismatch')
        raw = gzip.decompress(data)
        if len(raw) != item['original_bytes'] or sha(raw) != item['original_sha256']:
            raise ValueError('original input mismatch')
        bank = json.loads(raw)
        if bank['name'] != item['bank_name'] or len(bank['presets']) != item['presets']:
            raise ValueError('bank identity/count mismatch')
        supplied.append(bank)
    prefix = 'Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/PresetBanks/'
    with zipfile.ZipFile(io.BytesIO(ab)) as z:
        members = set()
        for item in z.infolist():
            if not item.filename.startswith(prefix) or not item.filename.endswith('.superj6.bank.json'):
                continue
            if item.filename in members:
                raise ValueError('duplicate archive member')
            members.add(item.filename)
            raw = z.read(item)
            bank = json.loads(raw)
            # Only the explicitly identified user bank is outside factory scope.
            if bank['name'] == 'User Bank':
                continue
            bundled.append(bank)
            inputs.append({'member': item.filename, 'bytes': len(raw), 'sha256': sha(raw)})
    result = compare(supplied, bundled)
    if len(supplied) != manifest['total_banks'] or result['presets'] != manifest['total_presets']:
        raise ValueError('manifest totals mismatch')
    result.update(manifest_sha256=sha(mb), archive_sha256=sha(ab),
                  supplied_banks=len(supplied), bundled_banks=len(bundled), bundled_inputs=inputs)
    return result


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--manifest', type=Path, required=True)
    p.add_argument('--archive', type=Path, required=True)
    p.add_argument('--archive-sha256', required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    result = run(a.root, a.manifest, a.archive, a.archive_sha256)
    a.output.write_text(json.dumps(result, indent=2) + '\n')
    raise SystemExit(result['status'] != 'PASS')
