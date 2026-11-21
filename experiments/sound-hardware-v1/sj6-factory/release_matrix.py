#!/usr/bin/env python3
"""Select the S4 schedule directly from the original build-9 application bundle."""
import argparse
from hashlib import sha256
from io import BytesIO
import json
from pathlib import Path
from zipfile import ZipFile

from factory import canonical_bytes
from original.audit_bundled_banks import AU, IPA, PREFIX

HERE = Path(__file__).resolve().parent
EXECUTABLE = 'Payload/SuperJ6.app/PlugIns/SuperJ6 AU.appex/SuperJ6 AU'


def json_bytes(obj):
    return canonical_bytes(obj) + b'\n'


def prepare_release(ipa):
    """Use only the exact release bytes, preserving its UUIDs, metadata and controls."""
    raw = ipa.read_bytes()
    if sha256(raw).hexdigest() != IPA:
        raise ValueError('expected the original build-9 SuperJ6 IPA')
    with ZipFile(BytesIO(raw)) as archive:
        if sha256(archive.read(EXECUTABLE)).hexdigest() != AU:
            raise ValueError('unexpected original AU executable')
        banks = {}
        for member in archive.infolist():
            if member.filename.startswith(PREFIX) and member.filename.endswith('.superj6.bank.json'):
                bank = json.loads(archive.read(member))
                if bank['uuid'] in banks:
                    raise ValueError(f"ambiguous bundled bank {bank['uuid']}")
                banks[bank['uuid']] = bank
    return select_bundled(banks, (HERE / 's4-inputs.json').read_bytes(),
                          json.loads((HERE / 'original/bundled-bank-audit.json').read_bytes()))


def select_bundled(banks, schedule, audit):
    if (audit['spec_sha256'] != sha256(schedule).hexdigest() or
            audit['ipa_sha256'] != IPA or audit['au_sha256'] != AU):
        raise ValueError('original schedule and release audit disagree')
    spec = json.loads(schedule)
    spec['source_ipa_sha256'] = IPA
    spec['export_schedule_sha256'] = audit['spec_sha256']
    del spec['archive_sha256']
    entries = {row['uid']: row for row in audit['programs']}
    files = {}
    checked = []
    for row in spec['programs']:
        uid = row['program_uid']
        match = entries[uid]
        bank = banks[match['bundled_bank_uuid']]
        if bank['name'] != row['bank'] or match['supplied_bank_uuid'] != row['bank_uuid']:
            raise ValueError(f'wrong bundled bank for {uid}')
        records = [preset for preset in bank['presets'] if preset['uid'] == uid]
        if len(records) != 1 or records[0]['name'] != row['program']:
            raise ValueError(f'missing or ambiguous bundled program {uid}')
        preset = records[0]
        if len(preset['parameterValues']) != match['parameter_count']:
            raise ValueError(f'wrong number of controls for {uid}')
        item = {'bank': {key: value for key, value in bank.items() if key != 'presets'},
                'preset': preset}
        files[uid] = json_bytes(item)
        row.update(bank_uuid=bank['uuid'], position=preset['position'],
                   record_sha256=sha256(canonical_bytes(preset)).hexdigest())
        checked.append({'uid': uid, 'parameter_count': match['parameter_count'],
                        'bundled_bank_uuid': bank['uuid'], 'supplied_bank_uuid': bank['uuid'],
                        'file_sha256': sha256(files[uid]).hexdigest()})
    spec_raw = json_bytes(spec)
    receipt = {'ipa_sha256': IPA, 'au_sha256': AU, 'component_version': audit['component_version'],
               'spec_sha256': sha256(spec_raw).hexdigest(), 'programs': checked}
    return spec_raw, receipt, files


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ipa', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    spec_raw, receipt, files = prepare_release(args.ipa)
    args.output.mkdir(parents=True, exist_ok=False)
    for uid, content in files.items():
        (args.output / f'{uid}.json').write_bytes(content)
    (args.output / 's4-inputs.json').write_bytes(spec_raw)
    (args.output / 'bundled-bank-audit.json').write_bytes(json_bytes(receipt))
    print(f"Selected {len(files)} original build-9 programs")
