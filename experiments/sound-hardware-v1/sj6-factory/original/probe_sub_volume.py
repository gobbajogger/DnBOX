"""Run the original build-9 ID 202 setter until the Objective-C output boundary."""
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from unicorn import UC_ARCH_ARM64, UC_HOOK_CODE, UC_MODE_ARM, Uc
from unicorn.arm64_const import (
    UC_ARM64_REG_D0, UC_ARM64_REG_D1, UC_ARM64_REG_LR, UC_ARM64_REG_PC,
    UC_ARM64_REG_S0, UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X20,
)

BASE = 0x100000000
STOP = 0x200000000
SET_SUB = 0x1000ECF74
OBJC_RETAIN = 0x1005D99FC
POW = 0x1005D9A98
MASTER_VOLUME_MSG = 0x1005E4A00
ORIGINAL_SHA = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
ARCHIVE_SHA = 'abe2f2e089306770489fb30c8870f608dc2f2e0804541f624337523679638a28'
BANK_SHA = 'ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'


def digest(s):
    return hashlib.sha256(s).hexdigest()


def dword(v):
    return struct.unpack('<Q', struct.pack('<d', v))[0]


def dvalue(v):
    return struct.unpack('<d', struct.pack('<Q', v))[0]


def fword(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fvalue(v):
    return struct.unpack('<f', struct.pack('<I', v))[0]


def main(archive, bank, result):
    assert digest(archive.read_bytes()) == ARCHIVE_SHA
    with ZipFile(archive) as inputs:
        binary = inputs.read('SuperJ6 AU.appex/SuperJ6 AU')
    assert digest(binary) == ORIGINAL_SHA
    assert digest(bank.read_bytes()) == BANK_SHA
    with ZipFile(bank) as inputs:
        records = [
            json.loads(inputs.read(member)) for member in inputs.namelist()
            if member.startswith('J6presets/') and member.endswith('.superj6.bank.json')
        ]
    assert len(records) == 12
    words = []
    for bank_row in records:
        for preset in bank_row['presets']:
            fields = [field['value'] for field in preset['parameterValues']
                      if field['identifier'] == 'neonsubVolume']
            assert len(fields) == 1
            words.append(fword(fields[0]))
    assert len(words) == 640

    # The original Objective-C stub loads its selector through a chained rebase.
    selector = struct.unpack_from('<Q', binary, 0x776c18)[0] & ((1 << 36) - 1)
    assert binary[selector:binary.index(b'\0', selector)] == b'setMasterVolume:'
    assert struct.unpack_from('<Q', binary, 0x7bee38)[0] == 0x88

    machine = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    machine.mem_map(BASE, 0x80c000)
    machine.mem_write(BASE, binary[:0x7f4000])
    machine.mem_map(STOP, 0x10000)

    # Test doubles for the three Swift objects read by the setter. Use the
    # original sub property offset and replace only the unloaded owner offset.
    outer, engine, instrument, sub_node = [STOP + n for n in (0x1000, 0x2000, 0x3000, 0x4000)]
    machine.mem_write(0x100809a40, struct.pack('<Q', 0x18))
    machine.mem_write(outer + 0x18, struct.pack('<Q', engine))
    machine.mem_write(engine + 0x20, struct.pack('<Q', instrument))
    machine.mem_write(instrument + 0x88, struct.pack('<Q', sub_node))
    observed_imports = []

    def import_hook(uc, pc, _size, _data):
        if pc == POW:
            x = dvalue(uc.reg_read(UC_ARM64_REG_D0))
            y = dvalue(uc.reg_read(UC_ARM64_REG_D1))
            assert y == 1.0
            observed_imports.append('pow')
            uc.reg_write(UC_ARM64_REG_D0, dword(math.pow(x, y)))
        else:
            assert pc == OBJC_RETAIN and uc.reg_read(UC_ARM64_REG_X0) == sub_node
            observed_imports.append('objc_retain')
        uc.reg_write(UC_ARM64_REG_PC, uc.reg_read(UC_ARM64_REG_LR))

    for address in (OBJC_RETAIN, POW):
        machine.hook_add(UC_HOOK_CODE, import_hook, begin=address, end=address)

    def measure(raw):
        machine.reg_write(UC_ARM64_REG_SP, STOP + 0x8000)
        machine.reg_write(UC_ARM64_REG_LR, STOP)
        machine.reg_write(UC_ARM64_REG_X20, outer)
        machine.reg_write(UC_ARM64_REG_S0, raw)
        observed_imports.clear()
        machine.emu_start(SET_SUB, MASTER_VOLUME_MSG, count=200)
        assert machine.reg_read(UC_ARM64_REG_PC) == MASTER_VOLUME_MSG
        assert machine.reg_read(UC_ARM64_REG_X0) == sub_node
        assert observed_imports == ['objc_retain', 'pow']
        return machine.reg_read(UC_ARM64_REG_D0)

    # Run every authored input, not merely a sampled distribution.
    all_outputs = [(word, measure(word)) for word in words]
    assert all(output == dword(0.808 * fvalue(word) + 0.0) for word, output in all_outputs)
    points = []
    for raw in (0x80000000, 0, 0x3e800000, 0x3f000000, 0x3f400000, 0x3f800000):
        output = measure(raw)
        assert output == dword(0.808 * fvalue(raw) + 0.0)
        points.append({'raw_word': f'{raw:08x}', 'binary64_output_word': f'{output:016x}',
                       'output': dvalue(output), 'if_rounded_to_binary32': f'{fword(dvalue(output)):08x}'})
    result.write_text(json.dumps({
        'result': 'PASS',
        'scope': 'Original ARM64 setter to sub-node Objective-C setMasterVolume: dispatch. No synth rendering.',
        'original_archive_sha256': ARCHIVE_SHA, 'original_executable_sha256': ORIGINAL_SHA,
        'factory_archive_sha256': BANK_SHA,
        'source': {'control_id': 202, 'control_name': 'neonsubVolume', 'preset_position': 24,
                   'property_descriptor': '0x100809a98', 'selected_setter': hex(SET_SUB),
                   'captured_message': 'setMasterVolume:', 'capture_pc': hex(MASTER_VOLUME_MSG)},
        'setup': 'Synthetic Swift owner pointers, original sub node offset; objc_retain identity, host libm pow, stop before Objective-C output method.',
        'factory_programs': len(words), 'distinct_raw_words': len(set(words)),
        'authored_zero_words': Counter(words)[0] + Counter(words)[0x80000000],
        'binary64_matches_0_808_times_binary32_raw': len(all_outputs),
        'ordered_factory_input_and_output_sha256': digest(b''.join(struct.pack('<IQ', *pair) for pair in all_outputs)),
        'selected_cases': points,
    }, indent=2) + '\n')
    print('PASS: original sub-node setter, 640/640 authored words and six controls match binary64 0.808 × raw')


if __name__ == '__main__':
    main(*map(Path, sys.argv[1:]))
