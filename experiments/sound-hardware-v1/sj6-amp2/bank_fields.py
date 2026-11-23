"""Decode factory identifiers once, preserving legacy and globally owned values."""
from collections import Counter
from hashlib import sha256
import json
from pathlib import Path
import struct
from zipfile import ZipFile

HERE = Path(__file__).resolve().parent
CATALOG_PATH = "experiments/sound-hardware-v1/mapping/results/implementer_brief/public_control_catalog.json"
CATALOG_SHA = "e1be22d91887954da614cf197ef98680d0f21dfc8b9ca9bec1dba0a304f6e711"

def decode_factory(repo, bank_zip):
    catalog_bytes = (repo / CATALOG_PATH).read_bytes()
    if sha256(catalog_bytes).hexdigest() != CATALOG_SHA:
        raise ValueError("SuperJ6 app control catalog digest changed")
    catalog = json.loads(catalog_bytes)
    controls = {p["identifier"]: p for p in catalog["controls"]}
    unknown_names = set(catalog["unknown_legacy_keys"])
    hashes = json.loads((HERE / "bank-hashes.json").read_text())
    seen_programs = set()
    unknown_counts = Counter()
    raw_roles = Counter()
    input_hash = sha256()
    output_hash = sha256()
    payload = bytearray()
    with ZipFile(bank_zip) as z:
        banks = sorted(p for p in z.namelist() if p.startswith("J6presets/") and p.endswith(".superj6.bank.json"))
        if sorted(sha256(z.read(p)).hexdigest() for p in banks) != hashes:
            raise ValueError("The twelve original SuperJ6 bank digests must match")
        for p in banks:
            bank = json.loads(z.read(p))
            for record in bank["presets"]:
                key = (bank["uuid"], record["uid"])
                if key in seen_programs:
                    raise ValueError("Duplicate bank and preset identity")
                seen_programs.add(key)
                known = []
                unknown = {}
                names = set()
                for field in record["parameterValues"]:
                    name = field["identifier"]
                    if name in names:
                        raise ValueError("Duplicate preset field " + name)
                    names.add(name)
                    bits = struct.pack("=f", field["value"])
                    input_hash.update(name.encode() + b"\0" + bits)
                    if name in controls:
                        entry = controls[name]
                        known.append((entry["id"], bits))
                        raw_roles[entry["role"]] += 1
                    elif name in unknown_names:
                        unknown[name] = bits
                        unknown_counts[name] += 1
                    else:
                        raise ValueError("Uncatalogued input field " + name)
                # Round trip through ID lookup, including unknown named fields.
                # Original positions are used only to check losslessness here.
                restored = {catalog["controls"][i]["identifier"]: bits for i, bits in known}
                restored.update(unknown)
                if set(restored) != names:
                    raise ValueError("Factory field identity was lost")
                for f in record["parameterValues"]:
                    name = f["identifier"]
                    output_hash.update(name.encode() + b"\0" + restored[name])
                payload.extend(struct.pack("=H", len(known)))
                for control, bits in known:
                    payload.extend(struct.pack("=B", control))
                    payload.extend(bits)
    if len(seen_programs) != 640 or input_hash.digest() != output_hash.digest():
        raise ValueError("Factory program or field round trip failed")
    report = {"banks": len(banks), "programs": len(seen_programs), "serialized_field_rows": sum(raw_roles.values()) + sum(unknown_counts.values()),
              "known_rows_by_role": dict(sorted(raw_roles.items())), "legacy_name_only": dict(sorted(unknown_counts.items())),
              "all_name_binary32_roundtrip_sha256": input_hash.hexdigest()}
    return payload, report
