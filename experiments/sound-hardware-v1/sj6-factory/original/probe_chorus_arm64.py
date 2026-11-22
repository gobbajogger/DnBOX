#!/usr/bin/env python3
"""Render original J60V2 kernel through a small ARM64 ABI harness."""
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE, UcError
from unicorn.arm64_const import (
    UC_ARM64_REG_D0, UC_ARM64_REG_LR, UC_ARM64_REG_PC, UC_ARM64_REG_S0,
    UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X2,
)

BASE = 0x100000000
SANDBOX = 0x200000000
RETURN = SANDBOX
OBJECT = SANDBOX + 0x1000
HEAP = SANDBOX + 0x10000
STACK = SANDBOX + 0x340000
CAPTURE = SANDBOX + 0x350000
STACK_GUARD = 0x1007319F8
CASES = [(1, 1., False), (1, 0., True), (1, 1., True), (2, 1., True), (3, 1., True)]
EXPECTED = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
FRAMES = 4096
RATE = 48000

def word(fmt, value):
    return struct.unpack('<' + ('I' if fmt == 'f' else 'Q'), struct.pack('<' + fmt, value))[0]

def sha(data):
    return hashlib.sha256(data).hexdigest()

def render(binary, stimulus, mode, mix, started, rate=RATE, frequency=1., depth=1.):
    u = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    u.mem_map(BASE, 0x800000)
    u.mem_write(BASE, binary[:0x7f4000])
    u.mem_map(SANDBOX, 0x400000)
    u.mem_write(STACK_GUARD, struct.pack('<Q', SANDBOX + 0x20))
    symbols = {
        0x1005d9450: 'allocate',
        0x1005d95dc: 'bzero',
        0x1005da020: 'tan',
        0x1005d9b7c: 'sin',
        0x1005d9600: 'cos',
        0x1005d98c4: 'memmove',
        0x1005d942c: 'release',
    }
    heap_next = HEAP
    def runtime(emu, address, _size, _user):
        nonlocal heap_next
        symbol = symbols[address]
        a, b = (emu.reg_read(k) for k in (UC_ARM64_REG_X0, UC_ARM64_REG_X1))
        if symbol == 'allocate':
            assert 0 < a <= 0x100000 and heap_next + a < STACK
            emu.reg_write(UC_ARM64_REG_X0, heap_next)
            heap_next += (a + 15) & ~15
        elif symbol == 'bzero':
            assert HEAP <= a <= a + b <= heap_next
            emu.mem_write(a, bytes(b))
        elif symbol == 'memmove':
            size = emu.reg_read(UC_ARM64_REG_X2)
            if size:
                assert HEAP <= a <= a + size <= heap_next and HEAP <= b <= b + size <= heap_next
                emu.mem_write(a, bytes(emu.mem_read(b, size)))
        elif symbol == 'release':
            assert not a or HEAP <= a < heap_next
        else:
            v = struct.unpack('<d', struct.pack('<Q', emu.reg_read(UC_ARM64_REG_D0)))[0]
            emu.reg_write(UC_ARM64_REG_D0, word('d', getattr(math, symbol)(v)))
        emu.reg_write(UC_ARM64_REG_PC, emu.reg_read(UC_ARM64_REG_LR))
    for address in symbols:
        u.hook_add(UC_HOOK_CODE, runtime, begin=address, end=address)

    def call(address, a=OBJECT, b=0, c=0, *, single=None, double=None):
        for register, value in [(UC_ARM64_REG_X0, a), (UC_ARM64_REG_X1, b), (UC_ARM64_REG_X2, c),
                                (UC_ARM64_REG_SP, STACK), (UC_ARM64_REG_LR, RETURN)]:
            u.reg_write(register, value)
        if single is not None:
            u.reg_write(UC_ARM64_REG_S0, word('f', single))
        if double is not None:
            u.reg_write(UC_ARM64_REG_D0, word('d', double))
        try:
            u.emu_start(address, RETURN, count=25_000_000)
        except UcError as exc:
            raise RuntimeError((hex(u.reg_read(UC_ARM64_REG_PC)), hex(u.reg_read(UC_ARM64_REG_LR)))) from exc
        assert u.reg_read(UC_ARM64_REG_PC) == RETURN, hex(u.reg_read(UC_ARM64_REG_PC))

    call(0x100004068)
    call(0x100004460, b=2, double=rate)
    call(0x10000422c, b=0, c=1, single=float(mode))
    call(0x10000422c, b=1, c=1, single=frequency)
    call(0x10000422c, b=2, c=1, single=depth)
    call(0x10000422c, b=3, c=1, single=mix)
    call(0x10000422c, b=4, c=1, single=0.0)
    readback = {}
    for key, param in [('mode', 0), ('frequency_scale', 1), ('depth_scale', 2), ('mix', 3), ('ramp_duration', 4)]:
        call(0x1000043cc, b=param)
        readback[key] = struct.unpack('<f', struct.pack('<I', u.reg_read(UC_ARM64_REG_S0)))[0]
    assert readback == {'mode': float(mode), 'frequency_scale': frequency, 'depth_scale': depth, 'mix': mix,
                        'ramp_duration': 0.}
    if started:
        u.mem_write(OBJECT + 0x29, b'\1')

    for list_address, channel in [(OBJECT + 0x100, CAPTURE), (OBJECT + 0x200, CAPTURE + 0x8000)]:
        u.mem_write(list_address, struct.pack('<IIIIQIIQ', 2, 0, 1, FRAMES * 4, channel,
                                               1, FRAMES * 4, channel + FRAMES * 4))
    u.mem_write(OBJECT + 0x18, struct.pack('<QQ', OBJECT + 0x100, OBJECT + 0x200))
    total = len(stimulus) // 8
    assert total % FRAMES == 0
    channels = [bytearray(), bytearray()]
    for start in range(0,total,FRAMES):
        for side in (0,1):
            offset=(side*total+start)*4
            u.mem_write(CAPTURE+side*FRAMES*4, stimulus[offset:offset+FRAMES*4])
        call(0x100004730, b=FRAMES)
        for side in (0,1):
            channels[side].extend(u.mem_read(CAPTURE+0x8000+side*FRAMES*4,FRAMES*4))
    return bytes(channels[0]+channels[1]), readback

def main():
    original = Path(sys.argv[1])
    out = Path(sys.argv[2])
    out.mkdir()
    with ZipFile(original) as container:
        binary = container.read('SuperJ6 AU.appex/SuperJ6 AU')
    assert sha(binary) == EXPECTED
    left = [0.25 if k == 0 else 0.0 for k in range(FRAMES)]
    right = [-0.125 if k == 37 else 0.0 for k in range(FRAMES)]
    stimulus = struct.pack(f'<{2 * FRAMES}f', *(left + right))
    (out / 'stimulus.f32').write_bytes(stimulus)
    values = []
    for mode, mix, started in CASES:
        rendered, readback = render(binary, stimulus, mode, mix, started)
        assert (rendered, readback) == render(binary, stimulus, mode, mix, started)
        name = f'mode{mode}-mix{mix:g}-start{int(started)}'
        (out / (name + '.f32')).write_bytes(rendered)
        data = struct.unpack(f'<{2 * FRAMES}f', rendered)
        assert all(map(math.isfinite, data))
        values.append(dict(case=name, reference_sha256=sha(rendered), dry_equal=rendered == stimulus,
                           params_readback=readback, started=started, left_energy=sum(x*x for x in data[:FRAMES]), right_energy=sum(x*x for x in data[FRAMES:])))
    print(json.dumps(dict(original_au_sha256=EXPECTED, layout='planar binary32 little endian',
                          stimulus_sha256=sha(stimulus), frames=FRAMES, rate=RATE, reference_runtime='Unicorn 2.1.4',
                          runner_sha256=sha(Path(__file__).read_bytes()), cases=values), indent=2))

if __name__ == '__main__':
    main()
