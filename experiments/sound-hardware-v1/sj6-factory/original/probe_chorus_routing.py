"""Bounded, comparison-only execution of original build-9 chorus routing."""
from collections import Counter
from hashlib import sha256
import json
import math
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm64_const import (
    UC_ARM64_REG_D0, UC_ARM64_REG_D1, UC_ARM64_REG_LR, UC_ARM64_REG_PC,
    UC_ARM64_REG_S0, UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X2, UC_ARM64_REG_X20,
)

BASE, MEMORY = 0x100000000, 0x200000000
EXE_SHA = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
IPA_ZIP_SHA = 'abe2f2e089306770489fb30c8870f608dc2f2e0804541f624337523679638a28'
BANK_SHA = 'ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'

def digest(data):
    return sha256(data).hexdigest()

def fw(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]

def ff(word):
    return struct.unpack('<f', struct.pack('<I', word))[0]

def dw(value):
    return struct.unpack('<Q', struct.pack('<d', value))[0]

def df(word):
    return struct.unpack('<d', struct.pack('<Q', word))[0]


def main(archive, banks, receipt):
    assert digest(archive.read_bytes()) == IPA_ZIP_SHA
    assert digest(banks.read_bytes()) == BANK_SHA
    with ZipFile(archive) as z:
        binary = z.read('SuperJ6 AU.appex/SuperJ6 AU')
    assert digest(binary) == EXE_SHA
    with ZipFile(banks) as z:
        programs = [
            {f['identifier']: f['value'] for f in p['parameterValues']}
            for name in z.namelist() if name.startswith('J6presets/') and name.endswith('.superj6.bank.json')
            for p in json.loads(z.read(name))['presets']
        ]
    assert len(programs) == 640

    u = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    u.mem_map(BASE, 0x80c000)
    u.mem_write(BASE, binary[:0x7f4000])
    u.mem_map(MEMORY, 0x10000)

    owner, engine, legacy, j60, booster, j60_au = [MEMORY + i*0x1000 for i in range(1, 7)]
    def wptr(address, value):
        u.mem_write(address, struct.pack('<Q', value))
    def ubyte(address):
        return int(u.mem_read(address, 1)[0])
    def wbyte(address, value):
        u.mem_write(address, bytes([value]))
    # Resolve only the unloaded main owner offset. In-object effect offsets,
    # shared mode offset, and nested J60V2 offsets are original data.
    wptr(0x100809a40, 0x18)
    wptr(owner + 0x18, engine)
    wptr(engine + 0x80, legacy)
    wptr(engine + 0x88, booster)
    wptr(engine + 0x90, j60)
    wptr(j60 + 0x08, j60_au)
    calls = []

    stubs = {
        0x1005d9c60: 'begin_access',
        0x1005d99fc: 'objc_retain', 0x1005d99f0: 'objc_release',
        0x1005d9ec4: 'swift_retain', 0x1005d9eac: 'swift_release',
        0x10044df68: 'legacy_off', 0x1003e8cbc: 'legacy_on',
        0x1005e3da0: 'booster_gain', 0x1005db7c0: 'start_observer',
        0x1005df540: 'au_ramp_query', 0x1005e5120: 'au_param',
        0x1005e51a0: 'au_param', 0x1005e70e0: 'j60_start',
        0x1005e7380: 'j60_stop', 0x1005d9a98: 'pow',
        0x1005e3d20: 'legacy_frequency', 0x1005e3500: 'legacy_depth',
        0x1005e3660: 'legacy_mix'
    }

    def substitute(machine, pc, _size, _data):
        symbol = stubs[pc]
        target = machine.reg_read(UC_ARM64_REG_X0)
        if symbol == 'pow':
            x = df(machine.reg_read(UC_ARM64_REG_D0))
            y = df(machine.reg_read(UC_ARM64_REG_D1))
            assert y in (1.0, 3.33)
            machine.reg_write(UC_ARM64_REG_D0, dw(math.pow(x, y)))
        elif symbol == 'au_ramp_query':
            assert target == j60_au
            machine.reg_write(UC_ARM64_REG_X0, 0)
        elif symbol == 'au_param':
            assert target == j60_au
            calls.append((symbol, machine.reg_read(UC_ARM64_REG_X2), ff(machine.reg_read(UC_ARM64_REG_S0))))
        elif symbol in ('j60_start', 'j60_stop'):
            assert target == j60_au
            calls.append((symbol,))
        elif symbol in ('legacy_on', 'legacy_off'):
            assert machine.reg_read(UC_ARM64_REG_X20) == legacy
            calls.append((symbol,))
        elif symbol in ('legacy_frequency', 'legacy_depth', 'legacy_mix'):
            assert target == legacy
            calls.append((symbol, df(machine.reg_read(UC_ARM64_REG_D0))))
        elif symbol == 'booster_gain':
            assert target == booster
        machine.reg_write(UC_ARM64_REG_PC, machine.reg_read(UC_ARM64_REG_LR))

    for address in stubs:
        u.hook_add(UC_HOOK_CODE, substitute, begin=address, end=address)

    def call(entry, *, raw=0., id=0, end=MEMORY):
        u.reg_write(UC_ARM64_REG_SP, MEMORY + 0xc000)
        u.reg_write(UC_ARM64_REG_LR, MEMORY)
        u.reg_write(UC_ARM64_REG_X20, owner)
        u.reg_write(UC_ARM64_REG_X0, id)
        u.reg_write(UC_ARM64_REG_S0, fw(raw))
        calls.clear()
        u.emu_start(entry, end, count=1500)
        assert u.reg_read(UC_ARM64_REG_PC) == end
        return list(calls)

    call(0x10001f374)
    version_default = ff(u.reg_read(UC_ARM64_REG_S0))
    assert version_default == 1.0

    for raw, expected in ((0., 0), (.49999997, 0), (.5, 1), (1., 1)):
        call(0x10000ae10, raw=raw, end=0x10000ae9c)
        assert ubyte(engine + 0x98) == expected

    switch_tested = 0
    for mode in range(4):
        for raw in (0., 0.0001, 1.):
            for entry, end, bit in ((0x1000f386c, 0x1000f38f8, 1), (0x1000f3c60, 0x1000f3cf0, 2)):
                wbyte(legacy + 0xc, mode)
                call(entry, raw=raw, end=end)
                assert ubyte(legacy + 0xc) == ((mode | bit) if raw > 0 else (mode & ~bit))
                switch_tested += 1

    defaults = {}
    for id, label, entry, address, scale in (
        (215, 'rate_scale', 0x10000ad50, 1, 2),
        (216, 'depth_scale', 0x10000ad68, 2, 2),
        (217, 'mix', 0x10000ad80, 3, 1),
    ):
        call(0x10001f32c, id=id)
        raw_default = ff(u.reg_read(UC_ARM64_REG_S0))
        assert raw_default == 1.0 / scale
        assert call(entry, raw=raw_default) == [('au_param', address, 1.0)]
        for raw in (0.0, .25, .5, .75, 1.0):
            assert call(entry, raw=raw) == [('au_param', address, scale * raw)]
        defaults[id] = {'control': label, 'raw_default': raw_default, 'au_parameter': address,
                        'au_default': 1.0, 'tested_endpoints': 5}

    legacy_defaults = {}
    for id, name, entry, leaf, symbol, multiplier, exponent in (
        (42, 'neonchorusRate', 0x1000f3d0c, 0x10001f4c0, 'legacy_frequency', 16.0, 3.33),
        (43, 'neonchorusDepth', 0x1000f3d9c, 0x10001f3c0, 'legacy_depth', .97, 1.0),
        (44, 'neonchorusMix', 0x1000f3e00, 0x10001f38c, 'legacy_mix', 1.0, 1.0),
    ):
        call(leaf)
        raw_default = ff(u.reg_read(UC_ARM64_REG_S0))
        values = Counter(fw(row[name]) if name in row else None for row in programs)
        words = {value for value in values if value is not None} | {fw(raw_default), 0, fw(1.0)}
        for raw in sorted(words):
            assert call(entry, raw=ff(raw)) == [(symbol, multiplier * math.pow(ff(raw), exponent))]
        legacy_defaults[id] = {'raw_default': raw_default, 'capture': symbol,
                               'transfer_multiplier': multiplier, 'transfer_exponent': exponent,
                               'effective_default': multiplier * math.pow(raw_default, exponent),
                               'authored_count': 640 - values[None], 'distinct_words_tested': len(words)}

    route_counts = Counter()
    version_counts = Counter()
    for row in programs:
        mode = int(row['neonchorusIEnable']) + 2 * int(row['neonchorusIIEnable'])
        version = row.get('neonchorusVersion', version_default)
        version_counts[str(row.get('neonchorusVersion'))] += 1
        for id, name in [(215, 'Rate'), (216, 'Depth'), (217, 'Mix')]:
            assert 'neonj60v2' + name not in row
        wbyte(legacy + 0xc, mode)
        call(0x10000ae10, raw=version, end=0x10000ae9c)
        route = call(0x10000ab30)
        if mode > 0 and version == 1:
            assert route == [('legacy_off',), ('au_param', 0, float(mode)), ('j60_start',)]
            route_counts[f'j60v2_mode_{mode}'] += 1
        elif mode > 0:
            assert route == [('legacy_on',), ('j60_stop',)]
            route_counts[f'legacy_mode_{mode}'] += 1
        else:
            assert route == [('legacy_off',), ('j60_stop',)]
            route_counts['bypass'] += 1
    # All eight synthetic combinations verify route 3 even though no release
    # row selects J60V2 mode 3.
    routes = {}
    for version in (0, 1):
        for mode in range(4):
            wbyte(legacy + 0xc, mode)
            call(0x10000ae10, raw=version, end=0x10000ae9c)
            routes[f'version{version}_mode{mode}'] = call(0x10000ab30)
    assert routes['version1_mode3'] == [('legacy_off',), ('au_param', 0, 3.0), ('j60_start',)]
    receipt.write_text(json.dumps({
        'original_executable_sha256': EXE_SHA, 'factory_bank_sha256': BANK_SHA,
        'scope': 'Original ARM64 control math and graph routing with synthetic owners. No original AU framework or factory instrument render.',
        'executed': {'factory_programs': len(programs), 'switch_transition_checks': switch_tested,
                     'version_threshold_tests': 4, 'j60_scalar_checks': 18},
        'version_normalized_default': version_default,
        'version_authored_counts': version_counts,
        'factory_routes': route_counts, 'synthetic_routes': routes, 'j60_parameter_defaults': defaults,
        'j60_parameter_authored_counts': {215: 0, 216: 0, 217: 0},
        'legacy_parameters_not_applied_to_j60': legacy_defaults,
    }, indent=2) + '\n')
    print('PASS: all 640 original chorus routes, switch transitions and exact J60V2 defaults')


if __name__ == '__main__':
    main(*map(Path, sys.argv[1:]))
