#!/usr/bin/env python3
"""Check factory command IDs and float words using the original save bindings and kernel defaults."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FRONTEND = ROOT / 'tests/fixtures/synthone-factory/frontend'
BANKS = ROOT / 'assets/factory_banks/synthone'
SWIFT = ROOT / 'tests/fixtures/synthone-tuning/upstream/SynthOne/AudioKitSynthOne/DSP/AKSynthOne.swift'


def blob_id(data):
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def pinned(path, expected):
    data = path.read_bytes()
    if blob_id(data) != expected:
        raise ValueError(f'upstream input changed: {path.name}')
    return data.decode()


def original_parameters(work):
    enum = pinned(HERE / 'original/S1Parameter.h', '3b0fcdfe82c58ac5b26bd31c7fddd78167045b18')
    kernel = pinned(HERE / 'original/S1DSPKernel.hpp', '175615d3fdd920df2054546dc1523a0ea8f0066e')
    names = dict((name, int(index)) for name, index in re.findall(r'^\s*(\w+) = (\d+)', enum, re.M))
    if names.pop('S1ParameterCount') != 150 or set(names.values()) != set(range(150)):
        raise ValueError('invalid original enum')
    rates = kernel[kernel.index('    const float minutesPerSecond'):kernel.index('    S1ParameterInfo s1p[')]
    table = kernel.split('    S1ParameterInfo s1p[', 1)[1].split('};', 1)[0]
    rows = re.findall(r'\{\s*\w+,\s*([^,]+),\s*([^,]+),\s*([^,]+),\s*"', table)
    if len(rows) != len(names):
        raise ValueError('unexpected kernel table')
    # defaultValue addresses the array by numeric slot, including rows with a repeated tag.
    # Compile its float expressions in C++ and retain the kernel's clamp.
    band_count = re.search(r'^#define S1_NUM_BANDLIMITED_FTABLES .+$', kernel, re.M)[0]
    cpp = work / 'defaults.cpp'
    cpp.write_text('#include <algorithm>\n#include <cstdio>\n' + band_count + '\nint main() {\n' + rates +
                   'const float rows[][3] = {\n' + ',\n'.join('{' + ','.join(row) + '}' for row in rows) +
                   '};\nfor (auto &r : rows) std::printf("%a\\n", '
                   'double(std::min(std::max(r[1],r[0]),r[2])));\n}\n')
    exe = work / 'defaults'
    subprocess.run(['c++', '-std=c++17', '-ffp-contract=off', cpp, '-o', exe], check=True)
    defaults = [float.fromhex(s) for s in subprocess.check_output([exe], text=True).splitlines()]
    return names, defaults


def original_bindings(names):
    manager = pinned(FRONTEND / 'PresetDataManager.swift', '9077cc36192f902f56620c35ae16ba5e57abeedc')
    save = manager.split('func saveValuesToPreset()', 1)[1]
    preset = pinned(FRONTEND / 'Preset.swift', 'ca03ec2995b96b7fb427a9eea52d9f7ae00c9863')
    bindings = {}
    for key, parameter in re.findall(r'activePreset\.(\w+)\s*=\s*(?:Int\()?s\.getSynthParameter\(\.(\w+)\)', save):
        fallback = re.search(rf'\b{key}\s*=\s*dictionary\["{key}"\]\s*as\?\s*\w+\s*\?\?[^\n]*?p\(\.(\w+)\)', preset)
        if not fallback:
            raise ValueError(f'missing dictionary binding for {key}')
        value = key, names[fallback[1]]
        if bindings.setdefault(names[parameter], value) != value:
            raise ValueError(f'ambiguous reverse mapping for {parameter}')
    if len(bindings) != 96:
        raise ValueError('expected 96 scalar fields')
    swift = SWIFT.read_text()
    steps = {}
    for key, getter in re.findall(r'activePreset\.(\w+)\[i\]\s*=\s*s\.(\w+)\(forIndex: i\)', save):
        body = swift.split('open func ' + getter + '(', 1)[1].split('\n    }', 1)[0]
        base = names[re.search(r'S1Parameter\.(\w+)\.rawValue', body)[1]]
        for step in range(16):
            steps[base + step] = key, step
    if len(steps) != 48 or steps.keys() & bindings.keys():
        raise ValueError('invalid sequence layout')
    return bindings, steps


def compare(stream, identities, bindings, steps, defaults):
    fields = bindings.keys() | steps.keys()
    by_bank = {}
    identifiers = Counter()
    bank_and_uid = Counter()
    missing = Counter()
    def read_word():
        return struct.unpack('<I', stream.read(4))[0]
    manifest = json.loads((BANKS / 'MANIFEST.json').read_bytes())
    expected_count = sum(bank['count'] for bank in manifest['files'])
    if read_word() != 0x53315043 or read_word() != expected_count or len(identities) != expected_count:
        raise ValueError('stream header or identity count differs')
    position = 0
    for bank in sorted(manifest['files'], key=lambda b: b['name']):
        bank_file = BANKS / bank['name']
        raw = bank_file.read_bytes()
        if hashlib.sha256(raw).hexdigest() != bank['sha256']:
            raise ValueError(f'changed bank: {bank_file}')
        records = json.loads(raw)
        if len(records) != bank['count'] or len({row['position'] for row in records}) != len(records):
            raise ValueError(f'bank position collision or count mismatch: {bank_file}')
        for index, record in enumerate(records):
            key = (bank['name'], index, record['uid'], record['name'])
            identity = identities[position]
            if key != tuple(identity[k] for k in ('bank', 'preset_index', 'uid', 'name')):
                raise ValueError(f'identity mismatch: {key}')
            identifiers[record['uid']] += 1
            bank_and_uid[(key[0], record['uid'])] += 1
            seen = set()
            if read_word() != len(fields):
                raise ValueError(f'missing commands: {key}')
            for _ in range(len(fields)):
                ident, word = struct.unpack('<I4s', stream.read(8))
                if ident not in fields or ident in seen:
                    raise ValueError(f'invalid or repeated parameter {ident} in {key}')
                seen.add(ident)
                if ident in steps:
                    field, step = steps[ident]
                    value = record[field][step]
                else:
                    field, fallback = bindings[ident]
                    if field not in record:
                        missing[field] += 1
                    value = record.get(field, defaults[fallback])
                if struct.pack('<f', value) != word:
                    raise ValueError(f'wrong float for {key}, {field} (ID {ident})')
            position += 1
        by_bank[bank['name']] = len(records)
    if stream.read(1):
        raise ValueError('trailing commands')
    return {'banks': by_bank, 'records': position, 'verified_pre_setter_words': position * len(fields),
            'scalar_writes_per_program': len(bindings), 'sequence_writes_per_program': len(steps),
            'source_default_uses': dict(sorted(missing.items())), 'distinct_uids': len(identifiers),
            'distinct_bank_uid_pairs': len(bank_and_uid), 'distinct_bank_position_pairs': position}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--commands', type=Path, help='Output directory of compile-synthone-factory.py')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        commands = args.commands or work / 'commands'
        if args.commands is None:
            subprocess.run(['python3', ROOT / 'tools/compile-synthone-factory.py', '--out', commands],
                           check=True, stdout=subprocess.DEVNULL)
        names, defaults = original_parameters(work)
        bindings, steps = original_bindings(names)
        rows = json.loads((commands / 'ORIGINAL-PRESET-COMMANDS.json').read_bytes())['cases']
        binary = commands / 'preset-commands.bin'
        with binary.open('rb') as stream:
            report = compare(stream, rows, bindings, steps, defaults)
        report['commands_sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
        print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
