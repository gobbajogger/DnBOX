#!/usr/bin/env python3
"""Freeze the diagnosis agent's selected scalar numeric results for compiled C.

Reads numbers only. Does not compile, render or evaluate a synthesizer.
"""
from pathlib import Path
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parents[1]


def word(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def main():
    source = ROOT / "inputs" / "scalar_results.json"
    data = json.loads(source.read_text())
    rows = data["selected_solver_checks"]["rows"]
    if len(rows) != 38:
        raise RuntimeError("Expected the declared 38 independent scalar points")
    lines = ["/* Numeric fixtures from independent diagnosis; no engine source. */",
             "#ifndef C_SCALAR_CASES_H", "#define C_SCALAR_CASES_H",
             "#define SCALAR_CASE_COUNT 38u",
             "typedef struct { uint32_t w, c, expected; } scalar_case;",
             "static const scalar_case scalar_cases[SCALAR_CASE_COUNT] = {"]
    for row in rows:
        lines.append("    {0x%08xu, 0x%08xu, 0x%08xu}," %
                     (word(row["w"]), word(row["c"]), word(row["binary32_six"])))
    lines += ["};", "#endif", ""]
    target = ROOT / "tests" / "scalar_cases.h"
    with target.open("x") as stream:
        stream.write("\n".join(lines))
    print(json.dumps({"numeric_source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                      "generated_header_sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
                      "points": len(rows)}))


if __name__ == "__main__":
    main()
