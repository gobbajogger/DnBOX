#!/usr/bin/env python3
"""Generate a read-only, random-access Synth One bank from the original ordered physical writes."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[4]
BANKS = ROOT / 'assets/factory_banks/synthone'


def c_array(out, ctype, name, values):
    out.write(f'static const {ctype} {name}[] = {{\n')
    for start in range(0, len(values), 16):
        out.write('    ' + ','.join(map(str, values[start:start + 16])) + ',\n')
    out.write('};\n')


def build(manifest, out):
    cases = json.loads(manifest.read_bytes())['cases']
    by_bank = {}
    for case in cases:
        by_bank.setdefault(case['bank'], {})[case['preset_index']] = case
    commands = []
    names = bytearray()
    bank_rows, name_offsets = [], []
    for entry in sorted(json.loads((BANKS / 'MANIFEST.json').read_bytes())['files'], key=lambda b: b['name']):
        bank = entry['name']
        raw = (BANKS / bank).read_bytes()
        if hashlib.sha256(raw).hexdigest() != entry['sha256']:
            raise ValueError(f'changed input bank {bank}')
        originals = sorted(enumerate(json.loads(raw)), key=lambda row: row[1]['position'])
        first = originals[0][1]['position']
        bank_rows.append((bank[:-5], first, len(commands), len(originals)))
        for offset, (index, preset) in enumerate(originals):
            case = by_bank[bank][index]
            if (preset['position'] != first + offset or preset['name'] != case['name'] or
                    preset['uid'] != case['uid'] or case['source_sha256'] != entry['sha256']):
                raise ValueError(f'program binding mismatch at {bank}, {index}')
            name_offsets.append(len(names))
            names.extend(preset['name'].encode('utf-8') + b'\0')
            commands.append(case['commands'])
    if len(commands) != len(cases) or len(cases) != 694 or len(names) > 65535:
        raise ValueError('unexpected corpus or name table size')

    order = [ident for ident, _ in commands[0]]
    if len(order) != 144 or len(set(order)) != len(order) or any(
            [ident for ident, _ in row] != order for row in commands):
        raise ValueError('factory command order changed')
    words = []
    fields = []
    encoded = [0] * len(commands)
    bit = 0
    for column, ident in enumerate(order):
        values = [struct.unpack('<I', struct.pack('<f', row[column][1]))[0] for row in commands]
        dictionary = sorted(set(values))
        lookup = {v: i for i, v in enumerate(dictionary)}
        width = (len(dictionary) - 1).bit_length()
        fields.append((len(words), bit, ident, width))
        words.extend(dictionary)
        for i, word in enumerate(values):
            encoded[i] |= lookup[word] << bit
        bit += width
    stride = (bit + 7) // 8
    if len(words) > 65535 or bit > 65535 or max(f[3] for f in fields) > 16:
        raise ValueError('column descriptor overflow')
    out.write(f'enum {{ ROW_BYTES = {stride}, PROGRAM_COUNT = {len(commands)} }};\n')
    out.write('static const struct bank banks[] = {\n')
    for bank, first, offset, count in bank_rows:
        out.write(f'    {{{json.dumps(bank)}, {first}, {offset}, {count}}},\n')
    out.write('};\nstatic const struct field fields[] = {\n')
    for row in fields:
        out.write('    {' + ', '.join(map(str, row)) + '},\n')
    out.write('};\n')
    c_array(out, 'uint32_t', 'words', [f'0x{x:08x}u' for x in words])
    c_array(out, 'uint16_t', 'name_offsets', name_offsets)
    c_array(out, 'unsigned char', 'names', names)
    # Two terminal bytes allow the final column to use the same unaligned-safe byte loads.
    c_array(out, 'uint8_t', 'rows', b''.join(r.to_bytes(stride, 'little') for r in encoded) + bytes(2))


if __name__ == '__main__':
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('manifest', type=Path)
    cli.add_argument('output', type=Path)
    args = cli.parse_args()
    with args.output.open('w') as stream:
        build(args.manifest, stream)
