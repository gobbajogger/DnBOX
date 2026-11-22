#!/usr/bin/env python3
"""Read a real factory control back from the original bounded sampler DSP."""
from argparse import ArgumentParser
from hashlib import sha256
import json
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from rebuild_engine_entry import BANK_SHA, EXE_SHA, UID

BENCH_SHA = "c1b7e99bf4f3054f114557a66c5ad8060525a5f8065e3ff3c7d208dbe2d9bf9c"
BANK_UID = "22F572EB-EDCA-4321-B3C5-ED2B14FDE68B"
RECORD_SHA = "d7281f4528a3cfd66ba72784d8b2f1744cfbc5752f7191b69492a29a256dc791"


def verify_bench(archive, directory):
    if sha256(archive.read_bytes()).hexdigest() != BENCH_SHA:
        raise ValueError("Wrong original bounded bench archive")
    names = ("sampler_dsp.py", "macho_dsp.py", "safe_arm64.py", "SAMPLER-DEPENDENCY-CLOSURE.private.json")
    with ZipFile(archive) as inputs:
        for name in names:
            if (directory / name).read_bytes() != inputs.read(name):
                raise ValueError(f"Wrong extracted bench member: {name}")


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--banks", type=Path, required=True)
    parser.add_argument("--bench-archive", type=Path, required=True)
    parser.add_argument("--bench-directory", type=Path, required=True)
    args = parser.parse_args()
    verify_bench(args.bench_archive, args.bench_directory)
    if sha256(args.banks.read_bytes()).hexdigest() != BANK_SHA:
        raise ValueError("Wrong factory bank export")
    sys.path[:0] = [str(args.bench_directory.resolve()),
                    str(args.repo.resolve() / "experiments/sound-hardware-v1/sj6-factory")]
    from factory import canonical_bytes, load_factory, select
    from sampler_dsp import SamplerDSP
    from unicorn.arm64_const import UC_ARM64_REG_S0, UC_ARM64_REG_X2

    record = select(load_factory(args.banks), BANK_UID, UID)["preset"]
    if sha256(canonical_bytes(record)).hexdigest() != RECORD_SHA:
        raise ValueError("Wrong authored Medieval Sky record")
    authored, = (row["value"] for row in record["parameterValues"] if row["identifier"] == "neonfilterSustain")
    sampler = SamplerDSP()
    if sha256(sampler.image.b).hexdigest() != EXE_SHA:
        raise ValueError("Wrong original executable")
    obj, _ = sampler.call(0x10037CE9C, seconds=6., instructions=6000000)
    if sampler.allocations[obj] != 1440:
        raise ValueError("Unexpected native sampler object extent")
    sampler.sd(0, 44100.)
    status, _ = sampler.call(0x10037D2E4, x0=obj, x1=2, seconds=6., instructions=6000000)
    if status:
        raise ValueError(f"Native sampler init failed: {status}")
    core = obj + 56
    getter, setter = 0x1003710E8, 0x100371090
    sampler.allowed.append((getter, 0x1003710F4))

    def readback():
        sampler.call(getter, x0=core)
        return sampler.uc.reg_read(UC_ARM64_REG_S0)

    before = readback()
    expected, = struct.unpack("<I", struct.pack("<f", authored))
    sampler.sf(0, authored)
    sampler.uc.reg_write(UC_ARM64_REG_X2, 1)
    sampler.call(0x10037D308, x0=obj, x1=18)
    via_dsp = readback()
    sampler.sf(0, 1.)
    sampler.call(setter, x0=core)
    restored = readback()
    sampler.sf(0, authored)
    sampler.call(setter, x0=core)
    via_core = readback()
    if not before == restored == 0x3F800000 or not via_dsp == via_core == expected != before:
        raise ValueError("Original filter sustain did not round-trip as authored")
    print(json.dumps({"source": {"executable_sha256": EXE_SHA, "bench_sha256": BENCH_SHA,
                                 "export_record_sha256": RECORD_SHA, "uid": UID},
                      "sample_rate": 44100, "channels": 2, "component": "AKSamplerDSP",
                      "authored_field": "neonfilterSustain", "internal_dsp_address": 18,
                      "factory_graph_route_verified": False, "factory_preset_applied": False,
                      "before_float_bits": hex(before), "authored_float_bits": hex(expected),
                      "dsp_readback_float_bits": hex(via_dsp), "restored_float_bits": hex(restored),
                      "direct_core_readback_float_bits": hex(via_core),
                      "complete_graph_instances": 0, "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
