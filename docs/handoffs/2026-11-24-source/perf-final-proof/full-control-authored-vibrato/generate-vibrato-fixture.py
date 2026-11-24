#!/usr/bin/env python3
"""Create ephemeral exact-value test inputs from the authorized local bank."""
from hashlib import sha256
from pathlib import Path
import struct
import sys

source, archive, output = map(Path, sys.argv[1:])
if sha256(archive.read_bytes()).hexdigest() != "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8":
    raise SystemExit("unexpected authorized bank digest")
sys.path.insert(0, str(source / "experiments/sound-hardware-v1/sj6-factory"))
from factory import load_factory
from compile_bank import compile_bank

names = ('neonvibratoSemitones', 'neonvibratoRate')
records = []
for _, (_, bank) in sorted(load_factory(archive).items()):
    for record in sorted(bank.values(), key=lambda item: item['position']):
        first = {}
        for field in record['parameterValues']:
            first.setdefault(field['identifier'], field['value'])
        records.append(tuple(struct.unpack('<I', struct.pack('<f', first[name]))[0] for name in names))
assert len(records) == 640
assert all(0 <= struct.unpack('<f', struct.pack('<I', v))[0] <= 1 for row in records for v in row)
with (output / 'authored_vibrato.inc').open('w') as f:
    f.write('static const uint32_t authored_words[][2] = {\n')
    f.writelines(f'    {{0x{depth:08x}u, 0x{rate:08x}u}},\n' for depth, rate in records)
    f.write('};\n')
print('accepted archive, exact 105/106 words', len(records) * 2)
print('positive-zero depth/rate words', *(sum(row[i] == 0 for row in records) for i in range(2)))
print(compile_bank(archive, output / 'bank_data.inc'))
