#!/usr/bin/env python3
"""Compare all exported and shipped program records without changing either bank."""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct
from zipfile import ZipFile

from audit_bundled_banks import AU, IPA, PREFIX, sha

EXPORT = 'ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'


def banks(archive, prefix):
    loaded = {}
    for name in archive.namelist():
        if name.startswith(prefix) and name.endswith('.superj6.bank.json'):
            bank = json.loads(archive.read(name))
            if bank['name'] in loaded:
                raise ValueError(f"Duplicate bank name {bank['name']}")
            loaded[bank['name']] = bank
    return loaded


def indexed(records, field):
    result = {row[field]: row for row in records}
    if len(result) != len(records):
        raise ValueError(f'Duplicate {field} in a bank or program')
    return result


def compare_fields(original, supplied):
    a, b = original['parameterValues'], supplied['parameterValues']
    old = indexed(a, 'identifier')
    new = indexed(b, 'identifier')
    missing = sorted(old.keys()-new.keys())
    extra = sorted(new.keys()-old.keys())
    changed = {}
    spelling = []
    for name in sorted(old.keys() & new.keys()):
        av, bv = old[name]['value'], new[name]['value']
        aw, bw = struct.pack('<f', av), struct.pack('<f', bv)
        if aw != bw:
            changed[name] = [aw.hex(), bw.hex()]
        elif av != bv:
            spelling.append(name)
    metadata = {k: {'release': original.get(k), 'export': supplied.get(k)}
                for k in sorted(original.keys()|supplied.keys()) if k != 'parameterValues'
                and (k not in original or k not in supplied or original[k] != supplied[k])}
    return {'same_order': list(old) == list(new), 'missing_ids': missing, 'added_ids': extra,
            'binary32_changes': changed, 'equal_float_decimal_changes': spelling, 'metadata_changes': metadata}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('ipa', type=Path)
    p.add_argument('banks', type=Path)
    args = p.parse_args()
    if sha(args.ipa.read_bytes()) != IPA or sha(args.banks.read_bytes()) != EXPORT:
        raise ValueError('Unexpected input archive')
    with ZipFile(args.ipa) as archive:
        if sha(archive.read('Payload/SuperJ6.app/PlugIns/SuperJ6 AU.appex/SuperJ6 AU')) != AU:
            raise ValueError('Unrecognized AU')
        release = banks(archive, PREFIX)
    with ZipFile(args.banks) as archive:
        exported = banks(archive, 'J6presets/')
    totals = Counter()
    decimal = Counter()
    metadata = Counter()
    details = []
    summaries = []
    extras = []
    for name in sorted(release.keys()|exported.keys()):
        src, dst = release.get(name), exported.get(name)
        old = indexed(src['presets'] if src else [], 'uid')
        new = indexed(dst['presets'] if dst else [], 'uid')
        summary = {'name': name, 'release_bank_uuid': src['uuid'] if src else None,
                   'export_bank_uuid': dst['uuid'] if dst else None, 'release_programs': len(old),
                   'export_programs': len(new), 'shared_uids': len(old.keys() & new.keys())}
        summary['bank_metadata_changes'] = {k: [src.get(k), dst.get(k)] for k in sorted(src.keys()|dst.keys())
                    if k not in ('presets','uuid') and src.get(k) != dst.get(k)} if src and dst else {}
        summaries.append(summary)
        for uid in sorted(old.keys() ^ new.keys()):
            extras.append({'bank': name, 'uid': uid, 'name': (old.get(uid) or new[uid])['name'],
                           'present_only_in': 'release' if uid in old else 'export'})
        for uid in sorted(old.keys() & new.keys()):
            result = compare_fields(old[uid], new[uid])
            fields = result['metadata_changes']
            totals['shared_programs'] += 1
            totals['ordered_export_fields'] += len(new[uid]['parameterValues'])
            totals['same_order_and_all_float32_words'] += int(result['same_order'] and not result['binary32_changes'])
            totals['all_nonparameter_metadata_equal'] += int(not fields)
            totals['record_objects_equal'] += int(old[uid] == new[uid])
            totals['programs_with_equal_float_decimal_changes'] += int(bool(result['equal_float_decimal_changes']))
            decimal.update(result['equal_float_decimal_changes'])
            metadata.update(fields.keys())
            if fields or not result['same_order'] or result['missing_ids'] or result['added_ids'] or result['binary32_changes']:
                details.append({'uid': uid, 'bank': name, **{k:v for k,v in result.items() if k != 'equal_float_decimal_changes' and (k == 'same_order' or v)}})
    print(json.dumps({'ipa_sha256': IPA, 'au_sha256': AU, 'export_zip_sha256': EXPORT, 'totals': totals,
                      'equal_float_decimal_changes_by_field': decimal, 'metadata_changes_by_field': metadata,
                      'banks': summaries, 'unpaired_programs': extras, 'program_differences': details},
                     indent=2, allow_nan=False))


if __name__ == '__main__':
    main()
