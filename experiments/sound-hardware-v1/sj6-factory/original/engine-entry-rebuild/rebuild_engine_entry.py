#!/usr/bin/env python3
"""Authenticate a factory stimulus and execute the original build-9 initializer prefix."""
import argparse
import hashlib
from importlib import metadata
from io import BytesIO
import json
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

IPA_RECOVERY_SHA = "3cc86adee1b4b8a0516306f02d6ffeb2e828144565a824e75624212998366592"
IPA_SHA = "dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e"
IPA_MEMBER = "Payload/SuperJ6.app/PlugIns/SuperJ6 AU.appex/SuperJ6 AU"
AU_SHA = "abe2f2e089306770489fb30c8870f608dc2f2e0804541f624337523679638a28"
EXE_SHA = "a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1"
BANK_SHA = "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
AU_MEMBER = "SuperJ6 AU.appex/SuperJ6 AU"
UID = "87E999B2-447E-4B2E-9EA9-AE53F4DEE44B"
INIT, FIRST_CALL = 0x100059AEC, 0x100059B14
UNSUPPORTED_IMPORT = "_$s10Foundation4UUIDVMa"
UNICORN_VERSION = "2.1.4"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def require_digest(path, expected):
    data = path.read_bytes()
    if sha(data) != expected:
        raise ValueError(f"{path.name}: SHA-256 mismatch, expected {expected}, got {sha(data)}")
    return data


def arm64_bl_target(pc, instruction):
    if instruction >> 26 != 0x25:
        raise ValueError("Initializer call is not an immediate ARM64 BL")
    offset = instruction & ((1 << 26) - 1)
    if offset & (1 << 25):
        offset -= 1 << 26
    return pc + offset * 4


def macho_tables(data):
    magic, cpu_type, _, _, count, commands_size = struct.unpack_from("<6I", data)
    if magic != 0xFEEDFACF or cpu_type != 0x0100000C:
        raise ValueError("Expected a 64-bit ARM64 Mach-O")
    if 32 + commands_size > len(data):
        raise ValueError("Truncated Mach-O load commands")
    segments, stub_section, symbols, indirect, fixups = [], None, None, None, None
    offset = 32
    for _ in range(count):
        command, size = struct.unpack_from("<2I", data, offset)
        if size < 8 or offset + size > 32 + commands_size:
            raise ValueError("Invalid Mach-O load command size")
        if command == 0x19:
            address, mem_size, file_offset, file_size, _, protection, sections, _ = struct.unpack_from(
                "<4Q4I", data, offset + 24
            )
            if file_offset + file_size > len(data) or file_size > mem_size:
                raise ValueError("Invalid Mach-O segment extent")
            if address and mem_size:
                segments.append((address, mem_size, file_offset, file_size, protection))
            for i in range(sections):
                section = offset + 72 + 80 * i
                if data[section:section + 16].rstrip(b"\0") == b"__stubs":
                    address, length = struct.unpack_from("<2Q", data, section + 32)
                    first, stride = struct.unpack_from("<2I", data, section + 68)
                    stub_section = address, length, first, stride
        elif command == 2:
            symbols = struct.unpack_from("<4I", data, offset + 8)
        elif command == 0xB:
            indirect = struct.unpack_from("<2I", data, offset + 56)
        elif command == 0x80000034:
            fixups = struct.unpack_from("<2I", data, offset + 8)
        offset += size
    if not segments or stub_section is None or symbols is None or indirect is None:
        raise ValueError("Missing Mach-O segments, stubs, or symbol tables")
    return segments, stub_section, symbols, indirect, fixups


def chained_import(data, segments, fixups, address):
    def locate(guest, length):
        for start, _, offset, size, _ in segments:
            if start <= guest and guest + length <= start + size:
                return start, offset, size
        raise ValueError("Chained pointer or stub is outside file-backed segments")

    start, offset, _ = locate(address, 12)
    adrp, ldr, branch = struct.unpack_from("<3I", data, offset + address - start)
    if adrp & 0x9F00001F != 0x90000010 or ldr & 0xFFC003FF != 0xF9400210 or branch != 0xD61F0200:
        raise ValueError("Unsupported chained stub instructions")
    imm = ((adrp >> 5) & 0x7FFFF) * 4 + ((adrp >> 29) & 3)
    if imm & (1 << 20):
        imm -= 1 << 21
    pointer = (address & ~0xFFF) + (imm << 12) + ((ldr >> 10) & 0xFFF) * 8
    start, offset, size = locate(pointer, 8)
    chain = chained_pointer(data, segments[0][0], start, offset, size, pointer, fixups)
    return chain


def chained_pointer(data, image_base, start, file_offset, file_size, pointer, fixups):
    base, size = fixups
    if base + size > len(data) or size < 28:
        raise ValueError("Invalid chained fixup extent")
    version, starts, imports, strings, count, entry_format, string_format = struct.unpack_from("<7I", data, base)
    if (version, entry_format, string_format) != (0, 1, 0):
        raise ValueError("Unsupported chained import table formats")
    if not 28 <= starts <= imports - 4 <= strings - 4 < size - 4 or count > (strings - imports) // 4:
        raise ValueError("Invalid chained import table offsets")
    seg_count, = struct.unpack_from("<I", data, base + starts)
    if starts + 4 + seg_count * 4 > imports:
        raise ValueError("Truncated chained segment table")
    for i in range(seg_count):
        entry, = struct.unpack_from("<I", data, base + starts + 4 + 4 * i)
        if not entry:
            continue
        segment = starts + entry
        if segment + 22 > imports:
            raise ValueError("Truncated chained segment header")
        extent, page_size, fmt, vm_offset, _, pages = struct.unpack_from("<IHHQIH", data, base + segment)
        if image_base + vm_offset != start:
            continue
        if fmt != 6 or not page_size or segment + extent > imports or 22 + 2 * pages > extent:
            raise ValueError("Unsupported chained segment pointer format or extent")
        page, displacement = divmod(pointer - start, page_size)
        if page >= pages:
            break
        head, = struct.unpack_from("<H", data, base + segment + 22 + page * 2)
        if head >= 0x8000:
            break
        page_base = page * page_size
        while head <= displacement and head + 8 <= min(page_size, file_size - page_base):
            word, = struct.unpack_from("<Q", data, file_offset + page_base + head)
            if head == displacement:
                ordinal = word & 0xFFFFFF
                if not word >> 63 or ordinal >= count:
                    raise ValueError("Invalid chained bind")
                item, = struct.unpack_from("<I", data, base + imports + 4 * ordinal)
                name = base + strings + (item >> 9)
                return data[name:data.index(0, name, base + size)].decode()
            stride = ((word >> 51) & 0xFFF) * 4
            if not stride:
                break
            head += stride
        break
    raise ValueError("Stub pointer is not an on-chain bind")


def import_at(data, tables, address):
    segments, (start, length, first, stride), (symoff, count, stroff, strsize), (indoff, indcount), fixups = tables
    if not stride or not start <= address < start + length or (address - start) % stride:
        raise ValueError("Initializer target is not an aligned imported stub")
    slot = first + (address - start) // stride
    if slot >= indcount:
        raise ValueError("Initializer import slot is outside the indirect symbol table")
    index, = struct.unpack_from("<I", data, indoff + 4 * slot)
    if index & 0xC0000000 and fixups is not None:
        return chained_import(data, segments, fixups, address)
    if index >= count:
        raise ValueError("Initializer import symbol is outside the symbol table")
    string_index, = struct.unpack_from("<I", data, symoff + 16 * index)
    return data[stroff + string_index:data.index(0, stroff + string_index, stroff + strsize)].decode()


def factory_program(repo, archive):
    sys.path.insert(0, str(repo / "experiments/sound-hardware-v1/sj6-factory"))
    from factory import canonical_bytes, load_factory, select
    source = repo / "experiments/sound-hardware-v1/sj6-factory"
    schedule = json.loads((source / "s4-inputs.json").read_text())
    item, = [row for row in schedule["programs"] if row["program_uid"] == UID]
    factory = load_factory(archive)
    preset = select(factory, item["bank_uuid"], UID)["preset"]
    preset_sha = sha(canonical_bytes(preset))
    if preset_sha != item["record_sha256"]:
        raise ValueError("Authenticated bank record does not match the scheduled UID and digest")
    return {"uid": UID, "record_sha256": preset_sha, "sample_rate": schedule["sample_rate"],
            "capture_frames": schedule["capture_frames"], "events": len(schedule["events"])}


def release_container_identity(release_banks, audit):
    if (audit["ipa_sha256"], audit["au_sha256"], audit["export_zip_sha256"]) != (IPA_SHA, EXE_SHA, BANK_SHA):
        raise ValueError("Archived bank audit input identity mismatch")
    expected = {bank["name"]: bank for bank in audit["banks"]}
    if expected.keys() != release_banks.keys():
        raise ValueError("Released bank names differ from the archived audit")
    counts = []
    for name, row in sorted(expected.items()):
        bank = release_banks[name]
        presets = bank["presets"]
        if bank["uuid"] != row["release_bank_uuid"] or len(presets) != row["release_programs"]:
            raise ValueError(f"Released bank identity or program count differs from the audit: {name}")
        ids = {preset["uid"] for preset in presets}
        if len(ids) != len(presets):
            raise ValueError(f"Duplicate release program UID in {name}")
        counts.append({"name": name, "release_uuid": bank["uuid"], "export_uuid": row["export_bank_uuid"],
                       "programs": len(presets)})
    factory_banks = [row for row in counts if row["export_uuid"] is not None]
    fields = sum(len(preset["parameterValues"]) for row in factory_banks
                 for preset in release_banks[row["name"]]["presets"])
    if (len(factory_banks), sum(row["programs"] for row in factory_banks), fields) != (12, 640, 90549):
        raise ValueError("Unexpected authenticated factory bank, program, or ordered control count")
    for row in audit["unpaired_programs"]:
        if row["present_only_in"] != "release":
            raise ValueError("Unexpected export-only unmatched program")
        if not any(preset["uid"] == row["uid"] for preset in release_banks[row["bank"]]["presets"]):
            raise ValueError("Release-only program UID absent from authenticated banks")
    return {"factory_bank_count": 12, "factory_program_count": 640, "authored_control_words": fields,
            "release_only_programs": audit["unpaired_programs"], "banks": counts}


def recovered_release(repo, recovery):
    source = repo / "experiments/sound-hardware-v1/sj6-factory"
    sys.path.insert(0, str(source))
    sys.path.insert(0, str(source / "original"))
    from audit_all_programs import banks
    from audit_bundled_banks import PREFIX
    from release_matrix import prepare_release, read_recovery_ipa
    ipa = read_recovery_ipa(recovery)
    with ZipFile(BytesIO(ipa)) as archive:
        image = archive.read(IPA_MEMBER)
        audit_bytes = (source / "original/all-programs.json").read_bytes()
        bank_identity = release_container_identity(banks(archive, PREFIX), json.loads(audit_bytes))
    bank_identity["audit_sha256"] = sha(audit_bytes)
    class ReleaseBytes:
        def is_dir(self):
            return False

        def read_bytes(self):
            return ipa

    schedule_bytes, _, records = prepare_release(ReleaseBytes())
    schedule = json.loads(schedule_bytes)
    row, = (row for row in schedule["programs"] if row["program_uid"] == UID)
    return image, {"uid": UID, "record_sha256": row["record_sha256"],
                   "release_item_sha256": sha(records[UID]),
                   "release_bank_uuid": row["bank_uuid"], "bank_identity": bank_identity,
                   "sample_rate": schedule["sample_rate"],
                   "capture_frames": schedule["capture_frames"], "events": len(schedule["events"])}


def execute_prefix(image):
    from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
    from unicorn.arm64_const import UC_ARM64_REG_LR, UC_ARM64_REG_PC, UC_ARM64_REG_SP

    tables = macho_tables(image)
    segments = tables[0]
    file_at_call = next(file_offset + FIRST_CALL - address for address, _, file_offset, size, _ in segments
                        if address <= FIRST_CALL < address + size)
    target = arm64_bl_target(FIRST_CALL, struct.unpack_from("<I", image, file_at_call)[0])
    name = import_at(image, tables, target)
    if name != UNSUPPORTED_IMPORT:
        raise ValueError(f"Original initializer import changed: {name}")
    machine = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    for address, size, file_offset, file_size, protection in segments:
        permissions = protection & 7
        page = address & ~0xFFF
        pages = (address + size + 0xFFF & ~0xFFF) - page
        machine.mem_map(page, pages)
        if file_size:
            machine.mem_write(address, image[file_offset:file_offset + file_size])
        machine.mem_protect(page, pages, permissions)
    stack = 0x200000000
    machine.mem_map(stack, 0x4000, 3)
    machine.reg_write(UC_ARM64_REG_SP, stack + 0x3000)
    machine.reg_write(UC_ARM64_REG_LR, 0)
    trace = []

    def record(guest, pc, _size, _data):
        if not INIT <= pc <= FIRST_CALL:
            raise ValueError(f"Unexpected guest instruction outside the admitted prefix: {pc:#x}")
        trace.append(pc)

    machine.hook_add(UC_HOOK_CODE, record)
    machine.emu_start(INIT, target, count=64)
    if machine.reg_read(UC_ARM64_REG_PC) != target or not trace or trace[-1] != FIRST_CALL:
        raise ValueError("Initializer did not reach the exact imported call boundary")
    return {"status": "BLOCKED_AT_REAL_FRAMEWORK_IMPORT", "initializer": hex(INIT),
            "call_site": hex(FIRST_CALL), "stub": hex(target), "import": name,
            "executed_original_instructions": len(trace)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--au", type=Path)
    source.add_argument("--ipa-recovery", type=Path)
    parser.add_argument("--banks", type=Path)
    args = parser.parse_args()
    if bool(args.au) != bool(args.banks):
        parser.error("--au requires --banks; --ipa-recovery supplies the original release banks")
    sources = (
        [("IPA recovery archive", args.ipa_recovery, IPA_RECOVERY_SHA)] if args.ipa_recovery else
        [("AU archive", args.au, AU_SHA), ("factory banks", args.banks, BANK_SHA)]
    )
    dependencies = []
    for label, path, pin in sources:
        if not path.is_file():
            dependencies.append({"dependency": label, "path": str(path), "sha256": pin})
    try:
        actual_version = metadata.version("unicorn")
    except metadata.PackageNotFoundError:
        actual_version = None
    if actual_version != UNICORN_VERSION:
        dependencies.append({"dependency": "Unicorn", "version": UNICORN_VERSION, "found": actual_version})
    result = {"complete_graph_instances": 0, "original_stereo_captures": 0, "qualified_s4_pairs": 0}
    if dependencies:
        result.update(status="MISSING_PINNED_INPUTS", dependencies=dependencies)
    else:
        if args.ipa_recovery:
            image, stimulus = recovered_release(args.repo.resolve(), args.ipa_recovery)
        else:
            archive_bytes = require_digest(args.au, AU_SHA)
            require_digest(args.banks, BANK_SHA)
            with ZipFile(BytesIO(archive_bytes)) as archive:
                image = archive.read(AU_MEMBER)
            stimulus = factory_program(args.repo.resolve(), args.banks)
        if sha(image) != EXE_SHA:
            raise ValueError("Embedded build-9 executable digest mismatch")
        result["factory_stimulus"] = stimulus
        result.update(execute_prefix(image))
    print(json.dumps(result, indent=2))
    return 2


if __name__ == "__main__":
    sys.exit(main())
