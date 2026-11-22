#!/usr/bin/env python3
"""Select the S4 schedule directly from the original build-9 application bundle."""
import argparse
from hashlib import sha256
from io import BytesIO
import json
import struct
from pathlib import Path
from zipfile import ZipFile

from factory import canonical_bytes
from original.audit_bundled_banks import AU, IPA, PREFIX

HERE = Path(__file__).resolve().parent
APP = 'Payload/SuperJ6.app/'
EXECUTABLE = APP + 'PlugIns/SuperJ6 AU.appex/SuperJ6 AU'


def json_bytes(obj):
    return canonical_bytes(obj) + b'\n'


def installed_banks(app):
    """Read the mobile bundle, or its standard macOS bundle layout."""
    extension = app.resolve() if app.suffix.lower() == '.appex' else None
    if extension is not None:
        app = next((parent for parent in extension.parents if parent.suffix.lower() == '.app'), None)
        if app is None:
            raise ValueError('the original AU also needs its containing app and factory banks')
    if app.suffix.lower() != '.app':
        raise ValueError('expected an installed SuperJ6 .app')
    # An iOS app installed on Apple Silicon can be inside a wrapper.
    if (app / 'Wrapper').is_dir():
        wrapped = list((app / 'Wrapper').glob('*.app'))
        if len(wrapped) != 1:
            raise ValueError('ambiguous wrapped SuperJ6 app')
        app = wrapped[0]
    if (app / 'Contents').is_dir():
        content = app / 'Contents'
        executable = content / 'PlugIns/SuperJ6 AU.appex/Contents/MacOS/SuperJ6 AU'
    else:
        content = app
        executable = app / EXECUTABLE.removeprefix(APP)
    if extension is not None and extension != (content / 'PlugIns/SuperJ6 AU.appex').resolve():
        raise ValueError('unexpected original AU extension')
    if not executable.is_file() or sha256(executable.read_bytes()).hexdigest() != AU:
        raise ValueError('unexpected original AU executable')
    framework = content / 'Frameworks/SuperJ6_Assets.framework'
    folders = {p.resolve() for p in (framework / 'PresetBanks', framework / 'Resources/PresetBanks',
                                     framework / 'Versions/Current/Resources/PresetBanks') if p.is_dir()}
    if len(folders) != 1:
        raise ValueError('missing or ambiguous installed factory bank directory')
    return [json.loads(p.read_bytes()) for p in sorted(folders.pop().glob('*.superj6.bank.json'))]


def prepare_release(source):
    """Select from the pinned IPA or authenticate an installed app's selected records."""
    metadata = None
    if source.is_dir():
        records = installed_banks(source)
        metadata = json.loads((HERE / 'original/all-programs.json').read_bytes())
        if metadata['ipa_sha256'] != IPA or metadata['au_sha256'] != AU:
            raise ValueError('wrong build-9 program audit')
    else:
        raw = source.read_bytes()
        if sha256(raw).hexdigest() != IPA:
            raise ValueError('expected the original build-9 SuperJ6 IPA')
        with ZipFile(BytesIO(raw)) as archive:
            if sha256(archive.read(EXECUTABLE)).hexdigest() != AU:
                raise ValueError('unexpected original AU executable')
            records = [json.loads(archive.read(member)) for member in archive.infolist()
                       if member.filename.startswith(PREFIX) and member.filename.endswith('.superj6.bank.json')]
    banks = {}
    for bank in records:
        if bank['uuid'] in banks:
            raise ValueError(f"ambiguous bundled bank {bank['uuid']}")
        banks[bank['uuid']] = bank
    return select_bundled(banks, (HERE / 's4-inputs.json').read_bytes(),
                          json.loads((HERE / 'original/bundled-bank-audit.json').read_bytes()), metadata)


def export_volume_values(value):
    """Candidate JSON numbers for the audited equal-binary32 export spelling."""
    packed = struct.pack('<f', value)
    single, = struct.unpack('<f', packed)
    yield value
    # The exported encoder can emit a short decimal or the exact widened Float.
    # A candidate is accepted only when the original SHA-256 also matches.
    for digits in range(1, 18):
        number = float(format(single, f'.{digits}g'))
        if struct.pack('<f', number) == packed:
            yield number
            if number.is_integer() and struct.pack('<f', int(number)) == packed:
                yield int(number)


def authenticate_installed(item, row, match, differences):
    """Rebuild the historically audited export only for its two SHA-256 checks."""
    uid = row['program_uid']
    export_bank = dict(item['bank'], uuid=row['bank_uuid'])
    preset = dict(item['preset'])
    changes = differences.get(uid, {}).get('metadata_changes', {})
    if set(changes) != set(match['metadata_differences']):
        raise ValueError(f'inconsistent release metadata audit for {uid}')
    for key, pair in changes.items():
        if key not in preset or preset[key] != pair['release']:
            raise ValueError(f'wrong installed {key} for {uid}')
        preset[key] = pair['export']

    def verify():
        return (sha256(canonical_bytes(preset)).hexdigest() == row['record_sha256'] and
                sha256(json_bytes({'bank': export_bank, 'preset': preset})).hexdigest() == match['file_sha256'])

    if not match['decimal_differences'] and verify():
        return
    if match['decimal_differences'] == ['neonvolumeControl']:
        fields = [dict(entry) for entry in preset['parameterValues']]
        preset['parameterValues'] = fields
        volumes = [entry for entry in fields if entry['identifier'] == 'neonvolumeControl']
        if len(volumes) == 1:
            volume = volumes[0]
            installed_value = volume['value']
            for value in export_volume_values(installed_value):
                if value == installed_value:
                    continue
                volume['value'] = value
                if verify():
                    return
    raise ValueError(f'installed bank or program does not match the audited record for {uid}')


def select_bundled(banks, schedule, audit, metadata=None):
    if (audit['spec_sha256'] != sha256(schedule).hexdigest() or
            audit['ipa_sha256'] != IPA or audit['au_sha256'] != AU):
        raise ValueError('original schedule and release audit disagree')
    spec = json.loads(schedule)
    if metadata is not None and metadata['export_zip_sha256'] != spec['archive_sha256']:
        raise ValueError('wrong export for the build-9 program audit')
    spec['source_ipa_sha256'] = IPA
    spec['export_schedule_sha256'] = audit['spec_sha256']
    del spec['archive_sha256']
    entries = {row['uid']: row for row in audit['programs']}
    files = {}
    checked = []
    differences = {r['uid']: r for r in metadata['program_differences']} if metadata else {}
    for row in spec['programs']:
        uid = row['program_uid']
        match = entries[uid]
        bank = banks.get(match['bundled_bank_uuid'])
        if bank is None:
            raise ValueError(f'missing original bundled bank for {uid}')
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
        if metadata is not None:
            authenticate_installed(item, row, match, differences)
        files[uid] = json_bytes(item)
        row.update(bank_uuid=bank['uuid'], position=preset['position'],
                   record_sha256=sha256(canonical_bytes(preset)).hexdigest())
        checked.append({'uid': uid, 'parameter_count': match['parameter_count'],
                        'bundled_bank_uuid': bank['uuid'], 'supplied_bank_uuid': bank['uuid'],
                        'file_sha256': sha256(files[uid]).hexdigest()})
    spec_raw = json_bytes(spec)
    receipt = {'ipa_sha256': IPA, 'au_sha256': AU, 'component_version': audit['component_version'],
               'spec_sha256': sha256(spec_raw).hexdigest(), 'programs': checked}
    if metadata is not None:
        receipt['source_kind'] = 'installed_app'
        receipt['reference_ipa_sha256'] = receipt.pop('ipa_sha256')
    return spec_raw, receipt, files


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, help='original build-9 IPA or installed .app directory')
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    spec_raw, receipt, files = prepare_release(args.source)
    args.output.mkdir(parents=True, exist_ok=False)
    for uid, content in files.items():
        (args.output / f'{uid}.json').write_bytes(content)
    (args.output / 's4-inputs.json').write_bytes(spec_raw)
    (args.output / 'bundled-bank-audit.json').write_bytes(json_bytes(receipt))
    print(f"Selected {len(files)} original build-9 programs")
