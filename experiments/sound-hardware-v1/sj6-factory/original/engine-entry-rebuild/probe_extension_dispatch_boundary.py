#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Follow the original extension factory through Objective-C's nil retain."""
from argparse import ArgumentParser
from io import BytesIO
import json
from pathlib import Path
from struct import pack
from zipfile import ZipFile

from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_LR, UC_ARM64_REG_PC, UC_ARM64_REG_X0, UC_ARM64_REG_X2

from probe_live_receiver_constructor import RETURN, STACK, guest
from rebuild_engine_entry import AU_MEMBER, AU_SHA, EXE_SHA, import_at, macho_tables, require_digest, sha

FACTORY = 0x10014D89C
RETAIN = 0x1005D99FC
DISPATCH = 0x1005D79BC
DISPATCH_SYMBOL = "_$s8Dispatch0A13WorkItemFlagsVMa"


def trace_factory(image, tables, receiver):
    machine = guest(image, tables)
    address = STACK + 0x1000
    machine.mem_write(address, pack("<5I", 0x61756D75, 0x6A617936, 0x41755072, 0, 0))
    machine.reg_write(UC_ARM64_REG_X0, receiver)
    machine.reg_write(UC_ARM64_REG_X2, address)
    stub_start, stub_length, _, _ = tables[1]
    imports, instructions = [], []

    def trace(current, pc, _size, _data):
        if stub_start <= pc < stub_start + stub_length:
            symbol = import_at(image, tables, pc)
            caller = current.reg_read(UC_ARM64_REG_LR) - 4
            resolved = (not imports and pc == RETAIN and symbol == "_objc_retain"
                        and current.reg_read(UC_ARM64_REG_X0) == 0)
            imports.append({"stub": hex(pc), "symbol": symbol,
                            "call_site": hex(caller), "nil_noop": resolved})
            if resolved:
                current.reg_write(UC_ARM64_REG_PC, caller + 4)
            else:
                current.emu_stop()
        else:
            instructions.append(pc)

    machine.hook_add(UC_HOOK_CODE, trace)
    machine.emu_start(FACTORY, RETURN, count=128)
    expected_imports = (["_objc_retain", DISPATCH_SYMBOL] if receiver == 0 else ["_objc_retain"])
    expected_endpoint = DISPATCH if receiver == 0 else RETAIN
    expected_count = 31 if receiver == 0 else 10
    if ([call["symbol"] for call in imports] != expected_imports
            or machine.reg_read(UC_ARM64_REG_PC) != expected_endpoint
            or len(instructions) != expected_count):
        raise ValueError("Original factory route ended at an unexpected instruction")
    return {"guest_instructions": len(instructions), "imports": imports}


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--au", type=Path, required=True)
    args = parser.parse_args()
    with ZipFile(BytesIO(require_digest(args.au, AU_SHA))) as archive:
        image = archive.read(AU_MEMBER)
    if sha(image) != EXE_SHA:
        raise ValueError("Original AU executable SHA-256 mismatch")
    tables = macho_tables(image)
    print(json.dumps({"au_sha256": EXE_SHA,
                      "factory_entry": hex(FACTORY),
                      "nil_receiver": trace_factory(image, tables, 0),
                      "nonobject_receiver": trace_factory(image, tables, 1),
                      "complete_graph_instances": 0,
                      "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
