#!/usr/bin/env python3
"""Probe the original Swift Conductor constructor before requiring a live object."""
import argparse
from io import BytesIO
import json
from pathlib import Path
from secrets import randbits
import struct
from zipfile import ZipFile

from unicorn.arm64_const import UC_ARM64_REG_X0, UC_ARM64_REG_X1, UC_ARM64_REG_X29

from probe_live_receiver_constructor import GUARD_SLOT, STACK, checked_segment, execute, guest
from rebuild_engine_entry import (
    AU_MEMBER, AU_SHA, EXE_SHA, chained_pointer, import_at, macho_tables, require_digest, sha,
)

MODEL_FACTORY = 0x1000BD780
MODEL_METADATA = 0x100256D5C
MODEL_DESCRIPTOR = 0x1006980B4
MODEL_INIT = 0x100259E08
SINGLETON_STUB = 0x1005D9DEC
DISPATCH_STUB = 0x1005D8034


def model_name(image, tables):
    def offset(address):
        start, file_at, _ = checked_segment(tables, address, 1)
        return file_at + address - start

    name_at = MODEL_DESCRIPTOR + 8
    relative, = struct.unpack_from("<i", image, offset(name_at))
    start = offset(name_at + relative)
    return image[start:image.index(0, start)].decode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--au", required=True, type=Path)
    args = parser.parse_args()
    with ZipFile(BytesIO(require_digest(args.au, AU_SHA))) as archive:
        image = archive.read(AU_MEMBER)
    if sha(image) != EXE_SHA:
        raise ValueError("Original AU executable identity changed")
    tables = macho_tables(image)
    name = model_name(image, tables)
    if name != "Conductor":
        raise ValueError("Model descriptor identity changed")
    metadata_vm = guest(image, tables)
    pcs = execute(metadata_vm, MODEL_FACTORY, SINGLETON_STUB)
    metadata_import = import_at(image, tables, SINGLETON_STUB)
    if (pcs[-1], metadata_vm.reg_read(UC_ARM64_REG_X0), metadata_vm.reg_read(UC_ARM64_REG_X1),
            MODEL_METADATA in pcs, metadata_import) != (
            0x100256D88, 0, MODEL_DESCRIPTOR, True, "_swift_getSingletonMetadata"):
        raise ValueError("Original Conductor metadata route changed")

    init_vm = guest(image, tables)
    slot = checked_segment(tables, GUARD_SLOT, 8)
    if chained_pointer(image, tables[0][0][0], *slot, GUARD_SLOT, tables[4]) != "___stack_chk_guard":
        raise ValueError("Original process guard binding changed")
    guard_address = STACK + 0x10000
    guard = struct.pack("<Q", randbits(64) or 1)
    init_vm.mem_map(guard_address, 0x1000, 3)
    init_vm.mem_write(guard_address, guard)
    init_vm.mem_protect(guard_address, 0x1000, 1)
    init_vm.mem_write(GUARD_SLOT, struct.pack("<Q", guard_address))
    init_pcs = execute(init_vm, MODEL_INIT, DISPATCH_STUB)
    dispatch_import = import_at(image, tables, DISPATCH_STUB)
    if init_pcs[-1] != 0x100259E50 or dispatch_import != "_$sSo17OS_dispatch_queueC8DispatchE20AutoreleaseFrequencyOMa":
        raise ValueError("Original direct constructor dependency changed")
    if init_vm.mem_read(init_vm.reg_read(UC_ARM64_REG_X29) - 0x70, 8) != guard:
        raise ValueError("Original model initializer did not copy the resolved process guard")
    print(json.dumps({"model": name, "au_sha256": EXE_SHA,
                      "factory_prefix": {"entry": hex(MODEL_FACTORY), "accessor": hex(MODEL_METADATA),
                                         "descriptor": hex(MODEL_DESCRIPTOR), "call_site": hex(pcs[-1]),
                                         "import": metadata_import, "guest_instructions": len(pcs)},
                      "initializer_prefix": {"entry": hex(MODEL_INIT), "call_site": hex(init_pcs[-1]),
                                             "import": dispatch_import, "guest_instructions": len(init_pcs),
                                             "guard_copy_verified": True},
                      "live_receiver_supplied": False, "complete_graph_instances": 0,
                      "original_stereo_captures": 0, "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
