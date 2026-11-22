#!/usr/bin/env python3
"""Verify and execute bounded original AU factory entry paths up to real imports."""
from argparse import ArgumentParser
import json
from pathlib import Path
import struct

from unicorn import UC_ARCH_ARM64, UC_HOOK_CODE, UC_MODE_ARM, Uc
from unicorn.arm64_const import UC_ARM64_REG_SP, UC_ARM64_REG_X0, UC_ARM64_REG_X2

from rebuild_engine_entry import import_at, macho_tables, recovered_release

CLASSES = {
    "_TtC10SuperJ6_AU23KingOfDigital_AudioUnit": ("factoryPresets", "setCurrentPreset:", "setFullState:",
                                               "allocateRenderResourcesAndReturnError:"),
    "_TtC10SuperJ6_AU24AKAUv3ExtensionAudioUnit": ("internalRenderBlock",),
}


def verify_methods(image, tables):
    sections = {}
    command = 32
    count, = struct.unpack_from("<I", image, 16)
    for _ in range(count):
        cmd, length = struct.unpack_from("<2I", image, command)
        if cmd == 0x19:
            nsects, = struct.unpack_from("<I", image, command + 64)
            for i in range(nsects):
                at = command + 72 + 80 * i
                name = image[at:at + 16].rstrip(b"\0").decode()
                sections[name] = struct.unpack_from("<QQI", image, at + 32)
        command += length
    image_base = tables[0][0][0]

    def offset(guest):
        for start, _, file_at, file_size, _ in tables[0]:
            if start <= guest < start + file_size:
                return file_at + guest - start
        raise ValueError("Address not file-backed")

    def rebase(guest, mask=0):
        word, = struct.unpack_from("<Q", image, offset(guest))
        if word >> 63:
            raise ValueError("Expected a local chained rebase")
        return image_base + ((word & 0xFFFFFFFFF) & ~mask)

    targets = {}
    classes, length, _ = sections["__objc_classlist"]
    for entry in range(classes, classes + length, 8):
        cls = rebase(entry)
        ro = rebase(cls + 32, 7)
        if not any(start <= ro < start + size for start, size, _ in
                   (sections["__objc_const"], sections["__objc_data"])):
            continue
        cname = offset(rebase(ro + 24))
        name = image[cname:image.index(0, cname)].decode()
        if name not in CLASSES:
            continue
        methods = rebase(ro + 32)
        flags, count = struct.unpack_from("<2I", image, offset(methods))
        if flags != 0x8000000C:
            raise ValueError("Unexpected Objective-C method encoding")
        found = {}
        for i in range(count):
            at = methods + 8 + 12 * i
            name_delta, _, implementation = struct.unpack_from("<3i", image, offset(at))
            selector = offset(rebase(at + name_delta))
            selector_name = image[selector:image.index(0, selector)].decode()
            if selector_name in CLASSES[name]:
                found[selector_name] = hex(at + 8 + implementation)
        if found.keys() != set(CLASSES[name]):
            raise ValueError(f"Missing expected methods in {name}")
        targets[name] = {"class": hex(cls), "methods_address": hex(methods), "implementations": found}
    if targets.keys() != CLASSES.keys():
        raise ValueError("Could not recover the expected AU classes")
    return targets


def first_import(image, tables, entry, stub):
    machine = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    for addr, size, offset, file_size, permissions in tables[0]:
        machine.mem_map(addr, size)
        machine.mem_write(addr, image[offset:offset + file_size])
        machine.mem_protect(addr, size, permissions)
    stack = 0x200000000
    machine.mem_map(stack, 0x4000, 3)
    machine.reg_write(UC_ARM64_REG_SP, stack + 0x3000)
    # Nonzero, unmapped markers only select the documented non-nil branch.
    # The test does not emulate a receiver or dictionary.
    machine.reg_write(UC_ARM64_REG_X0, 1)
    machine.reg_write(UC_ARM64_REG_X2, 2)
    trace = []

    def stop_at_import(guest, pc, _size, _data):
        trace.append(pc)
        start, length, _, _ = tables[1]
        if start <= pc < start + length:
            guest.emu_stop()

    machine.hook_add(UC_HOOK_CODE, stop_at_import)
    machine.emu_start(entry, 0, count=32)
    if not trace or trace[-1] != stub:
        raise ValueError("Original route did not reach the expected import")
    return {"call_site": hex(trace[-2]), "stub": hex(stub), "import": import_at(image, tables, stub),
            "executed_instructions": len(trace) - 1}


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--ipa-recovery", required=True, type=Path)
    args = parser.parse_args()
    image, stimulus = recovered_release(args.repo.resolve(), args.ipa_recovery)
    tables = macho_tables(image)
    classes = verify_methods(image, tables)
    implementations = classes["_TtC10SuperJ6_AU23KingOfDigital_AudioUnit"]["implementations"]
    calls = {"setCurrentPreset:": 0x1005D99FC, "setFullState:": 0x1005D7ADC}
    endpoints = {selector: first_import(image, tables, int(implementations[selector], 16), stub)
                 for selector, stub in calls.items()}
    print(json.dumps({"classes": classes, "first_non_nil_imports": endpoints,
                      "authenticated_factory_uid": stimulus["uid"], "factory_preset_applied": False,
                      "complete_graph_instances": 0, "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
