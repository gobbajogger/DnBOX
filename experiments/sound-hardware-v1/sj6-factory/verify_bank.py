#!/usr/bin/env python3
"""Check both C selectors against the entire ordered field stream for every program."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from uuid import UUID

from compile_bank import CATALOG, compile_bank
from factory import HERE, load_factory


class Cursor(c.Structure):
    _fields_ = [("bit", c.c_uint), ("field", c.c_uint), ("end", c.c_uint)]


def verify(archive, out):
    out.mkdir(parents=True, exist_ok=False)
    summary = compile_bank(archive, out / "bank_data.inc")
    subprocess.run(["cc", "-std=c11", "-Os", "-Wall", "-Wextra", "-Werror",
                    "-fPIC", "-shared", "-I" + str(out), HERE / "bank_reader.c",
                    "-o", out / "reader.so"], check=True)
    lib = c.CDLL(str(out / "reader.so"))
    lib.sj6_select_uid.argtypes = [c.c_char_p, c.c_char_p, c.POINTER(Cursor)]
    lib.sj6_select_position.argtypes = [c.c_uint, c.c_uint, c.POINTER(Cursor)]
    lib.sj6_next_field.argtypes = [c.POINTER(Cursor), c.POINTER(c.c_uint), c.POINTER(c.c_uint32)]
    factory = load_factory(archive)
    catalog = json.loads(CATALOG.read_bytes())
    names = [field["identifier"] for field in catalog["controls"]] + sorted(catalog["unknown_legacy_keys"])
    digest = hashlib.sha256()
    checks = 0
    for bank, (bank_id, (_, programs)) in enumerate(sorted(factory.items())):
        bank_bytes = UUID(bank_id).bytes
        for uid, record in programs.items():
            first, second = Cursor(), Cursor()
            if not lib.sj6_select_uid(bank_bytes, UUID(uid).bytes, c.byref(first)):
                raise ValueError(f"missing {bank_id}/{uid}")
            if not lib.sj6_select_position(bank, record["position"], c.byref(second)):
                raise ValueError(f"missing position {bank}/{record['position']}")
            if bytes(first) != bytes(second):
                raise ValueError(f"selectors disagree at {uid}")
            for expected in record["parameterValues"]:
                key, word = c.c_uint(), c.c_uint32()
                if not lib.sj6_next_field(c.byref(first), c.byref(key), c.byref(word)):
                    raise ValueError(f"early end at {uid}")
                bits = struct.pack("<I", word.value)
                if names[key.value] != expected["identifier"] or bits != struct.pack("<f", expected["value"]):
                    raise ValueError(f"field differs at {uid}/{expected['identifier']}")
                digest.update(names[key.value].encode() + b"\0" + bits)
                checks += 1
            if lib.sj6_next_field(c.byref(first), c.byref(key), c.byref(word)):
                raise ValueError(f"unexpected trailing field at {uid}")
        before = bytes(second)
        if (lib.sj6_select_position(bank, len(programs), c.byref(second)) or
                lib.sj6_select_uid(bank_bytes, bytes(16), c.byref(second)) or
                bytes(second) != before):
            raise ValueError("invalid selection modified the cursor")
    if checks != summary["fields"]:
        raise ValueError("incorrect total")
    summary.update(archive_sha256=hashlib.sha256(archive.read_bytes()).hexdigest(),
                   selected_by_uid_and_position=summary["programs"], ordered_binary32_checked=checks,
                   ordered_check_sha256=digest.hexdigest(), invalid_selections_rejected=2*len(factory))
    return summary


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.archive, args.output.resolve()), indent=2))
