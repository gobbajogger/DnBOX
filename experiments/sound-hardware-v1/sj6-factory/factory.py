#!/usr/bin/env python3
"""Select complete original SuperJ6 preset records for the engine-only bench."""
import argparse
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile

HERE = Path(__file__).resolve().parent


def canonical_bytes(record):
    return json.dumps(record, sort_keys=True, ensure_ascii=False, allow_nan=False,
                      separators=(",", ":")).encode("utf-8")


def load_factory(archive):
    """Index by original bank and program identity. Keep all authored fields."""
    expected = sorted(json.loads((HERE.parent / "sj6-amp2/bank-hashes.json").read_text()))
    by_id = {}
    digests = []
    with ZipFile(archive) as source:
        for member in source.infolist():
            if not (member.filename.startswith("J6presets/") and
                    member.filename.endswith(".superj6.bank.json")):
                continue
            raw = source.read(member)
            digests.append(hashlib.sha256(raw).hexdigest())
            bank = json.loads(raw)
            bank_id = bank["uuid"]
            if bank_id in by_id:
                raise ValueError(f"duplicate bank UUID {bank_id}")
            programs = {program["uid"]: program for program in bank["presets"]}
            if len(programs) != len(bank["presets"]):
                raise ValueError(f"duplicate preset UID in {bank_id}")
            by_id[bank_id] = ({k: v for k, v in bank.items() if k != "presets"}, programs)
    if sorted(digests) != expected:
        raise ValueError("expected the twelve authenticated factory bank members")
    return by_id


def select(factory, bank_uuid, program_uid):
    try:
        bank, programs = factory[bank_uuid.upper()]
        record = programs[program_uid.upper()]
    except KeyError as error:
        raise ValueError(f"unknown factory bank/program pair {bank_uuid}/{program_uid}") from error
    return {"bank": bank, "preset": record}


def materialize_matrix(factory, target):
    spec = json.loads((HERE / "s4-inputs.json").read_text())
    selected = []
    for row in spec["programs"]:
        item = select(factory, row["bank_uuid"], row["program_uid"])
        if hashlib.sha256(canonical_bytes(item["preset"])).hexdigest() != row["record_sha256"]:
            raise ValueError(f"wrong complete record for {row['program']}")
        selected.append((row["program_uid"], canonical_bytes(item)))
    target.mkdir(exist_ok=False, parents=True)
    for uid, data in selected:
        (target / f"{uid}.json").write_bytes(data + b"\n")
    (target / "s4-inputs.json").write_bytes((HERE / "s4-inputs.json").read_bytes())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    cmd = parser.add_subparsers(dest="command", required=True)
    cmd.add_parser("list")
    one = cmd.add_parser("select")
    one.add_argument("bank_uuid")
    one.add_argument("program_uid")
    matrix = cmd.add_parser("matrix")
    matrix.add_argument("output", type=Path)
    args = parser.parse_args()
    factory = load_factory(args.archive)
    if args.command == "matrix":
        materialize_matrix(factory, args.output)
    elif args.command == "select":
        print(canonical_bytes(select(factory, args.bank_uuid, args.program_uid)).decode())
    else:
        for bank, programs in factory.values():
            for record in programs.values():
                print(json.dumps([bank["uuid"], record["uid"], bank["name"],
                                  record["position"], record["name"]], ensure_ascii=False))


if __name__ == "__main__":
    main()
