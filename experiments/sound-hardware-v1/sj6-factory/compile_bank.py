#!/usr/bin/env python3
"""Pack the authored binary32 fields in original order for the C bank reader."""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct
from uuid import UUID

from factory import HERE, load_factory

CATALOG = HERE.parent / "mapping/results/implementer_brief/public_control_catalog.json"


class BitWriter:
    def __init__(self):
        self.data = bytearray()
        self.value = 0
        self.bits = 0

    def put(self, value, width):
        self.value |= value << self.bits
        self.bits += width
        while self.bits >= 8:
            self.data.append(self.value & 255)
            self.value >>= 8
            self.bits -= 8

    def finish(self):
        if self.bits:
            self.data.append(self.value)
        return self.data


def coding(counts):
    # For each width choose its most frequent exact words. Reserve a code
    # for a literal only if a value falls outside the dictionary.
    words = sorted(counts, key=lambda w: (-counts[w], w))
    total = sum(counts.values())
    choices = []
    for width in range(9):
        entries = min(len(words), 1 << width)
        if entries < len(words):
            entries -= 1
        stored = words[:entries]
        cost = 32 * entries + total * width + (total - sum(counts[w] for w in stored)) * 32
        choices.append((cost, width, stored))
    _, width, words = min(choices, key=lambda v: (v[0], v[1]))
    return width, words


def array(out, ctype, name, values):
    out.write(f"static const {ctype} {name}[] = {{\n")
    for index in range(0, len(values), 16):
        out.write("    " + ", ".join(str(v) for v in values[index:index+16]) + ",\n")
    out.write("};\n")


def compile_bank(archive, output):
    factory = load_factory(archive)
    catalog = json.loads(CATALOG.read_bytes())
    ids = {p["identifier"]: p["id"] for p in catalog["controls"]}
    for name in sorted(catalog["unknown_legacy_keys"]):
        ids[name] = len(ids)  # bench-only IDs for the two unmapped names
    counts = [Counter() for _ in ids]
    records, banks, bank_uid, program_uid = [], [], bytearray(), bytearray()
    for uid, (_, programs) in sorted(factory.items()):
        bank_uid.extend(UUID(uid).bytes)
        banks.append(len(records))
        for position, record in enumerate(sorted(programs.values(), key=lambda r: r["position"])):
            if record["position"] != position:
                raise ValueError(f"missing or duplicate position in {uid}")
            program_uid.extend(UUID(record["uid"]).bytes)
            fields = []
            for field in record["parameterValues"]:
                field_id = ids[field["identifier"]]
                word, = struct.unpack("<I", struct.pack("<f", field["value"]))
                counts[field_id][word] += 1
                fields.append((field_id, word))
            records.append(fields)
    banks.append(len(records))
    layouts, shapes, params, layout_offset = {}, [], [], [0]
    widths, dictionary_base, dictionary, codes = [], [0], [], []
    for histogram in counts:
        width, words = coding(histogram)
        widths.append(width)
        dictionary.extend(words)
        dictionary_base.append(len(dictionary))
        codes.append({w: i for i, w in enumerate(words)})
    offsets, payload = [0], bytearray()
    for fields in records:
        schema = tuple(i for i, _ in fields)
        if schema not in layouts:
            layouts[schema] = len(layouts)
            params.extend(schema)
            layout_offset.append(len(params))
        shapes.append(layouts[schema])
        writer = BitWriter()
        for i, word in fields:
            code = codes[i].get(word)
            if code is None:
                writer.put(len(codes[i]), widths[i])
                writer.put(word, 32)
            else:
                writer.put(code, widths[i])
        payload.extend(writer.finish())
        offsets.append(len(payload))
    if len(dictionary) >= 65536 or len(payload) >= 1 << 24:
        raise ValueError("generated table exceeds native offsets")
    with output.open("w") as out:
        array(out, "uint8_t", "bit_width", widths)
        array(out, "uint16_t", "dict_start", dictionary_base)
        array(out, "uint32_t", "words", [f"0x{x:08x}u" for x in dictionary])
        array(out, "uint16_t", "bank_start", banks)
        array(out, "uint8_t", "bank_uid", bank_uid)
        array(out, "uint8_t", "program_uid", program_uid)
        array(out, "uint8_t", "shape", shapes)
        array(out, "uint16_t", "layout_start", layout_offset)
        array(out, "uint8_t", "field_ids", params)
        array(out, "uint8_t", "record_start", b"".join(v.to_bytes(3, "little") for v in offsets))
        array(out, "uint8_t", "bits", payload)
    return dict(programs=len(records), banks=len(banks)-1, fields=sum(map(len, records)),
                layouts=len(layouts), payload_bytes=len(payload), dictionary_words=len(dictionary))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    print(json.dumps(compile_bank(args.archive, args.output)))
