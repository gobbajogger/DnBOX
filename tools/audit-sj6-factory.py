#!/usr/bin/env python3
"""Check preserved factory inputs against identity metadata, never claim DSP binding.

The corpus is supplied explicitly. Reports include hashes/counts/identities, not
preset values. Unknown fields are retained in inputs and reported, not discarded.
"""
import argparse
import collections
import gzip
import hashlib
import json
import math
from pathlib import Path


def digest(data):
    return hashlib.sha256(data).hexdigest()


def audit(root, manifest, schema):
    known = {p['identifier'] for p in schema['parameters']}
    result = {'scope': 'Input integrity and identity coverage only; not DSP mapping',
              'dsp_mapping_status': 'NOT_ADMITTED', 'banks': [], 'unknown_identifiers': {},
              'total_banks': 0, 'total_presets': 0, 'parameter_entries': 0}
    unknown = collections.Counter()
    identifiers = set()
    seen_files = set()
    for item in manifest['banks']:
        name = item['file']
        if Path(name).name != name or name in seen_files:
            raise ValueError('invalid or duplicate bank path')
        seen_files.add(name)
        data = (root / name).read_bytes()
        if len(data) != item['gzip_bytes'] or digest(data) != item['gzip_sha256']:
            raise ValueError('compressed input hash/length mismatch: ' + name)
        raw = gzip.decompress(data)
        if len(raw) != item['original_bytes'] or digest(raw) != item['original_sha256']:
            raise ValueError('original input hash/length mismatch: ' + name)
        bank = json.loads(raw)
        if bank['name'] != item['bank_name'] or len(bank['presets']) != item['presets']:
            raise ValueError('bank identity/count mismatch: ' + name)
        count = 0
        for preset in bank['presets']:
            seen = set()
            for entry in preset['parameterValues']:
                ident, value = entry['identifier'], entry['value']
                if not isinstance(ident, str) or not ident or ident in seen:
                    raise ValueError('invalid/duplicate parameter identity: ' + name)
                if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
                    raise ValueError('non-finite/non-numeric parameter: ' + name)
                seen.add(ident)
                identifiers.add(ident)
                count += 1
                if ident not in known:
                    unknown[ident] += 1
        result['banks'].append({'file': name, 'presets': item['presets'],
                                'original_sha256': digest(raw), 'parameter_entries': count})
        result['total_presets'] += item['presets']
        result['parameter_entries'] += count
    result['total_banks'] = len(result['banks'])
    if result['total_banks'] != manifest['total_banks'] or result['total_presets'] != manifest['total_presets']:
        raise ValueError('corpus totals mismatch')
    result['unknown_identifiers'] = dict(sorted(unknown.items()))
    result['distinct_identifiers'] = len(identifiers)
    result['recognized_identifiers'] = len(identifiers & known)
    result['input_integrity'] = 'PASS'
    result['complete_identity_coverage'] = 'FAIL' if unknown else 'PASS'
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--banks', type=Path, required=True)
    p.add_argument('--manifest', type=Path, required=True)
    p.add_argument('--schema', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    args = p.parse_args()
    result = audit(args.banks, json.loads(args.manifest.read_text()), json.loads(args.schema.read_text()))
    result['manifest_sha256'] = digest(args.manifest.read_bytes())
    result['schema_sha256'] = digest(args.schema.read_bytes())
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'banks'}, sort_keys=True))
    return 1 if result['complete_identity_coverage'] == 'FAIL' else 0


if __name__ == '__main__':
    raise SystemExit(main())
