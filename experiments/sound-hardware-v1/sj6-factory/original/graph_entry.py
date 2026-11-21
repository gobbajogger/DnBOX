#!/usr/bin/env python3
"""Observe the first runtime call made by the AU's KoFMInstrument initializer."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

INIT = 0x100059AEC
CALL = 0x100059B14
EXPECTED = 'a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('replay', type=Path)
    args = p.parse_args()
    sys.path.insert(0, str((args.replay / 'superj6-reference-bench-20261115').resolve()))
    from macho_dsp import DSP
    from safe_arm64 import BenchError
    from unicorn.arm64_const import UC_ARM64_REG_PC
    guest = DSP()
    if hashlib.sha256(guest.image.b).hexdigest() != EXPECTED:
        raise ValueError('Unexpected AU image')
    # Stop at the first linked framework dependency. No synthetic object, callback
    # success, or graph construction is supplied. The prefix saves x20 without dereferencing it.
    instr = guest.image.i(CALL)
    if instr >> 26 != 0x25:
        raise ValueError('Expected a direct ARM64 BL')
    imm = instr & 0x3ffffff
    if imm & (1 << 25):
        imm -= 1 << 26
    stub = CALL + (imm << 2)
    guest.allowed.extend([(INIT, CALL + 4), (stub, stub + 12)])
    try:
        guest.call(INIT)
    except BenchError as error:
        attempted = [d['name'] for d in guest.import_defs if d['address'] == guest.uc.reg_read(UC_ARM64_REG_PC)]
        if len(attempted) != 1 or attempted[0] not in str(error):
            raise
        result = {'au_sha256': EXPECTED, 'instrument': 'KoFMInstrument',
                  'initializer': hex(INIT), 'call_site': hex(CALL), 'import_stub': hex(stub),
                  'reached_import': attempted[0], 'exception': str(error),
                  'guest_basic_blocks_entered': guest.blocks, 'full_graph_instances': 0}
        (args.replay / 'instrument-init.json').write_text(json.dumps(result, indent=2)+'\n')
        print(json.dumps(result))
    else:
        raise RuntimeError('Unexpected initializer return')


if __name__ == '__main__':
    main()
