#!/usr/bin/env python3
"""Match the selected input files to the original release's factory records."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
from zipfile import ZipFile

IPA = 'dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e'
AU = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
PREFIX = 'Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/PresetBanks/'


def words(record):
    return [(entry['identifier'], struct.pack('<f', entry['value']))
            for entry in record['parameterValues']]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('ipa', type=Path)
    p.add_argument('selected', type=Path)
    args = p.parse_args()
    if sha(args.ipa.read_bytes()) != IPA:
        raise ValueError('IPA does not match reference build 9')
    spec_bytes = (args.selected/'s4-inputs.json').read_bytes()
    spec = json.loads(spec_bytes)
    with ZipFile(args.ipa) as archive:
        if sha(archive.read('Payload/SuperJ6.app/PlugIns/SuperJ6 AU.appex/SuperJ6 AU')) != AU:
            raise ValueError('AU image identity differs')
        banks = [json.loads(archive.read(name)) for name in archive.namelist()
                 if name.startswith(PREFIX) and name.endswith('.superj6.bank.json')]
    rows = []
    for row in spec['programs']:
        raw = (args.selected/(row['program_uid']+'.json')).read_bytes()
        item = json.loads(raw)
        supplied = item['preset']
        canonical = json.dumps(supplied, sort_keys=True, separators=(',', ':'), ensure_ascii=False,
                               allow_nan=False).encode()
        if sha(canonical) != row['record_sha256'] or item['bank']['uuid'] != row['bank_uuid']:
            raise ValueError('Selected record differs from S4 input')
        bank_matches = [bank for bank in banks if bank['name'] == row['bank']]
        if len(bank_matches) != 1:
            raise ValueError(f"Ambiguous bundled bank for {row['bank']}")
        bundled = bank_matches[0]
        matches = [record for record in bundled['presets'] if record['uid'] == row['program_uid']]
        if len(matches) != 1 or words(matches[0]) != words(supplied):
            raise ValueError(f"Bundled program or ordered Float32 controls differ for {row['program']}")
        actual = matches[0]
        rows.append({'uid': row['program_uid'], 'file_sha256': sha(raw),
                     'bundled_bank_uuid': bundled['uuid'], 'supplied_bank_uuid': row['bank_uuid'],
                     'parameter_count': len(words(actual)),
                     'decimal_differences': [a['identifier'] for a, b in zip(actual['parameterValues'],
                         supplied['parameterValues']) if a['value'] != b['value']],
                     'metadata_differences': sorted(k for k in set(actual) | set(supplied)
                         if k != 'parameterValues' and actual.get(k) != supplied.get(k))})
    print(json.dumps({'ipa_sha256': IPA, 'au_sha256': AU, 'component_version': 67072,
                      'spec_sha256': sha(spec_bytes), 'programs': rows}, indent=2))


if __name__ == '__main__':
    main()
