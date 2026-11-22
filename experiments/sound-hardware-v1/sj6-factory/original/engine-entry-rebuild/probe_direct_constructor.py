#!/usr/bin/env python3
"""Locate the first real import on two bounded original constructor routes."""
import argparse
import json
from pathlib import Path

from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_SP, UC_ARM64_REG_X20

from rebuild_engine_entry import import_at, macho_tables, recovered_release


def probe(image, tables, entry, expected_stub, placeholder_receiver):
    guest = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    for address, size, offset, length, protection in tables[0]:
        guest.mem_map(address, size)
        guest.mem_write(address, image[offset:offset + length])
        guest.mem_protect(address, size, protection)
    stack = 0x200000000
    guest.mem_map(stack, 0x8000, 3)
    guest.reg_write(UC_ARM64_REG_SP, stack + 0x4000)
    if placeholder_receiver:
        guest.reg_write(UC_ARM64_REG_X20, stack + 0x6000)
    trace = []

    def stop_at_stub(machine, pc, _size, _userdata):
        trace.append(pc)
        start, length, _, _ = tables[1]
        if start <= pc < start + length:
            machine.emu_stop()

    guest.hook_add(UC_HOOK_CODE, stop_at_stub)
    guest.emu_start(entry, 0, count=256)
    if trace[-1] != expected_stub:
        raise ValueError("Unexpected direct-constructor endpoint")
    return {"entry": hex(entry), "call_site": hex(trace[-2]), "stub": hex(trace[-1]),
            "import": import_at(image, tables, trace[-1]), "executed_instructions": len(trace) - 1,
            "receiver": "placeholder" if placeholder_receiver else "metadata request zero"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--ipa-recovery", type=Path, required=True)
    args = parser.parse_args()
    image, stimulus = recovered_release(args.repo.resolve(), args.ipa_recovery)
    tables = macho_tables(image)
    routes = [probe(image, tables, 0x1002AF36C, 0x1005D9DEC, False),
              probe(image, tables, 0x1002B07D8, 0x1005D79BC, True)]
    print(json.dumps({"authenticated_stimulus_uid": stimulus["uid"], "preset_applied": False,
                      "complete_graph_instances": 0, "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0, "routes": routes}, indent=2))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
