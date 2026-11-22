#!/usr/bin/env python3
"""Measure authentic AU constructor and post-bridge boundaries without runtime stand-ins."""
from argparse import ArgumentParser
import json
from pathlib import Path
from secrets import randbits
import struct

from unicorn import UC_ARCH_ARM64, UC_HOOK_CODE, UC_HOOK_MEM_WRITE_UNMAPPED, UC_MODE_ARM, Uc, UcError
from unicorn.arm64_const import (
    UC_ARM64_REG_LR, UC_ARM64_REG_PC, UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X29,
    UC_ARM64_REG_X2, UC_ARM64_REG_X20, UC_ARM64_REG_X8,
)

from rebuild_engine_entry import chained_pointer, import_at, macho_tables, recovered_release

STACK = 0x200000000
RETURN = STACK + 0x8000
GUARD_SLOT = 0x1007319F8


def checked_segment(tables, address, length):
    for start, _, file_offset, size, _ in tables[0]:
        if start <= address <= start + size - length:
            return start, file_offset, size
    raise ValueError("Original address is not file-backed")


def guest(image, tables):
    machine = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    for address, size, file_offset, length, protection in tables[0]:
        machine.mem_map(address, size)
        machine.mem_write(address, image[file_offset:file_offset + length])
        machine.mem_protect(address, size, protection)
    machine.mem_map(STACK, 0x4000, 3)
    machine.reg_write(UC_ARM64_REG_SP, STACK + 0x3000)
    machine.reg_write(UC_ARM64_REG_LR, RETURN)
    return machine


def execute(machine, entry, endpoint):
    pcs = []

    def trace(guest, pc, _size, _data):
        if pc == endpoint:
            guest.emu_stop()
        else:
            pcs.append(pc)

    machine.hook_add(UC_HOOK_CODE, trace)
    machine.emu_start(entry, RETURN, count=128)
    return pcs


def execute_with_guard(image, tables, description_bytes):
    machine = guest(image, tables)
    guard_slot = checked_segment(tables, GUARD_SLOT, 8)
    if chained_pointer(image, tables[0][0][0], *guard_slot, GUARD_SLOT, tables[4]) != "___stack_chk_guard":
        raise ValueError("Cannot resolve a slot without the authenticated guard binding")
    guard_page = STACK + 0x10000
    guard = randbits(64)
    while not guard:
        guard = randbits(64)
    guard_contents = struct.pack("<Q", guard)
    machine.mem_map(guard_page, 0x1000, 3)
    machine.mem_write(guard_page, guard_contents)
    machine.mem_protect(guard_page, 0x1000, 1)
    machine.mem_write(GUARD_SLOT, struct.pack("<Q", guard_page))
    description = STACK + 0x1000
    machine.mem_write(description, description_bytes)
    machine.reg_write(UC_ARM64_REG_X2, description)

    trace, faults = [], []
    machine.hook_add(UC_HOOK_CODE, lambda _machine, pc, _size, _context: trace.append(pc))

    def inaccessible_write(guest, _access, addr, size, value, _context):
        faults.append((guest.reg_read(UC_ARM64_REG_PC), addr, size, value))
        return False

    machine.hook_add(UC_HOOK_MEM_WRITE_UNMAPPED, inaccessible_write)
    try:
        machine.emu_start(0x1000BD9C8, RETURN, count=128)
    except UcError:
        if not faults:
            raise
    frame = machine.reg_read(UC_ARM64_REG_X29)
    if machine.mem_read(frame - 0x70, 8) != guard_contents:
        raise ValueError("Original prologue did not copy the bound process guard")
    expected_offset = machine.reg_read(UC_ARM64_REG_X8)
    if expected_offset != 0x60 or faults != [(0x1000BC7E4, 0x60, 8, 0)] or trace[-1] != 0x1000BC7E4:
        raise ValueError("Unexpected first constructor object dependency")
    return {"resolved_import": "___stack_chk_guard", "guard_copy_verified": True,
            "guest_instructions_before_unmapped_write": len(trace) - 1,
            "first_receiver_write": hex(faults[0][0]), "receiver_ivar_offset": hex(expected_offset),
            "receiver_constructed": False}


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--ipa-recovery", required=True, type=Path)
    args = parser.parse_args()
    image, stimulus = recovered_release(args.repo.resolve(), args.ipa_recovery)
    tables = macho_tables(image)

    constructor = guest(image, tables)
    description = STACK + 0x1000
    description_bytes = struct.pack("<5I", 0x61756D75, 0x6A617936, 0x41755072, 0, 0)
    constructor.mem_write(description, description_bytes)
    constructor.reg_write(UC_ARM64_REG_X2, description)
    # The zero receiver deliberately remains a non-object. Its first use
    # would be after the stop at the unresolved native data import.
    pcs = execute(constructor, 0x1000BD9C8, 0x1000BC7CC)
    slot = checked_segment(tables, GUARD_SLOT, 8)
    bind_name = chained_pointer(image, tables[0][0][0], *slot, GUARD_SLOT, tables[4])
    encoded, = struct.unpack_from("<Q", image, slot[1] + GUARD_SLOT - slot[0])
    if len(pcs) != 31 or pcs[-1] != 0x1000BC7C8 or constructor.reg_read(UC_ARM64_REG_X8) != encoded:
        raise ValueError("AU constructor reached a different original boundary")
    if bind_name != "___stack_chk_guard":
        raise ValueError("Wrong AU constructor data import")

    view = guest(image, tables)
    view.mem_write(description, description_bytes)
    view.reg_write(UC_ARM64_REG_X2, description)
    view_trace = execute(view, 0x10014D89C, 0x1005D99FC)
    if len(view_trace) != 10 or view_trace[-1] != 0x10014D8C0:
        raise ValueError("Unexpected original view factory entry")
    view_import = import_at(image, tables, 0x1005D99FC)
    if view_import != "_objc_retain":
        raise ValueError("Wrong view factory import")

    empty = guest(image, tables)
    nil_trace = execute(empty, 0x1000BF18C, RETURN)
    if len(nil_trace) != 11 or nil_trace[-1] != 0x1000BF264:
        raise ValueError("Unexpected original nil-state route")

    nonempty = guest(image, tables)
    # Nonzero markers select the native branch. Neither is dereferenced.
    nonempty.reg_write(UC_ARM64_REG_X0, 2)
    nonempty.reg_write(UC_ARM64_REG_X20, 1)
    nonnil_trace = execute(nonempty, 0x1000BF18C, 0x1000BF1B0)
    if len(nonnil_trace) != 9 or nonnil_trace[-1] != 0x1000BF1AC:
        raise ValueError("Unexpected original nonnil-state route")
    if nonempty.reg_read(UC_ARM64_REG_X8) != 0x138:
        raise ValueError("Unexpected receiver-owned lock offset")

    # The two local classes have on-disk metadata, but still need AudioToolbox's
    # superclass and Objective-C's real cache before receiver allocation.
    realization_bindings = {}
    for address, expected in (
        (0x100784990, "_OBJC_CLASS_$_AUAudioUnit"),
        (0x100784430, "__objc_empty_cache"),
        (0x100784998, "__objc_empty_cache"),
    ):
        segment = checked_segment(tables, address, 8)
        name = chained_pointer(image, tables[0][0][0], *segment, address, tables[4])
        if name != expected:
            raise ValueError("Unexpected original class realization binding")
        realization_bindings[hex(address)] = name
    print(json.dumps({"constructor": {"objc_entry": "0x1000bd9c8", "swift_entry": "0x1000bc788",
                                     "first_unresolved_dereference": "0x1000bc7cc",
                                     "data_slot": hex(GUARD_SLOT), "data_import": bind_name,
                                     "guest_instructions": len(pcs), "receiver_constructed": False},
                      "constructor_with_guard": execute_with_guard(image, tables, description_bytes),
                      "class_realization_bindings": realization_bindings,
                      "extension_factory": {"entry": "0x10014d89c", "call_site": "0x10014d8c0",
                                            "import": view_import, "guest_instructions": len(view_trace)},
                      "state_helper": {"entry": "0x1000bf18c", "nil_guest_instructions": len(nil_trace),
                                       "nonnil_guest_instructions": len(nonnil_trace),
                                       "first_receiver_access": "0x1000bf1b0", "lock_ivar_offset": "0x138"},
                      "authenticated_factory_uid": stimulus["uid"], "factory_preset_applied": False,
                      "complete_graph_instances": 0, "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
