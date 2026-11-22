#!/usr/bin/env python3
"""Replay authenticated real-sample sampler PCM through original chorus code."""
from argparse import ArgumentParser
from hashlib import sha256
import json
import math
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from rebuild_engine_entry import BANK_SHA, EXE_SHA, UID
from probe_sampler_authored_state import BANK_UID, RECORD_SHA, verify_bench

MATRIX_SHA = "08b912309b24edceebb159b8e3b1be1b3befc706201602212885ad73dd6cd3e0"
MATRIX_PREFIX = "superj6-original-asset-reference-20261115/matrix-v2-parent/"
FRAMES = 4096
SAMPLE_RATE = 48000


def main():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--banks", type=Path, required=True)
    parser.add_argument("--bench-archive", type=Path, required=True)
    parser.add_argument("--bench-directory", type=Path, required=True)
    parser.add_argument("--matrix", type=Path, required=True)
    args = parser.parse_args()
    verify_bench(args.bench_archive, args.bench_directory)
    if sha256(args.matrix.read_bytes()).hexdigest() != MATRIX_SHA:
        raise ValueError("Wrong bounded real-asset matrix")
    with ZipFile(args.matrix) as archive:
        result = json.loads(archive.read(MATRIX_PREFIX + "RESULT.private.json"))
        plan = json.loads(archive.read(MATRIX_PREFIX + "PLAN.private.json"))
        original = archive.read(MATRIX_PREFIX + "case-00-original.f32le.pcm")
    original_sha = sha256(original).hexdigest()
    if original_sha != result["cases"][0]["original"]["raw_sha256"] or plan["rate"] != SAMPLE_RATE:
        raise ValueError("Wrong archived original component PCM or clock")
    dry = original[:FRAMES * 8]

    def guarded(machine, payload):
        extent = len(payload)
        address = machine.alloc(extent + 64)
        machine.wr(address, b"G" * 32 + payload + b"G" * 32)
        return address + 32

    if sha256(args.banks.read_bytes()).hexdigest() != BANK_SHA:
        raise ValueError("Wrong authored banks")
    sys.path[:0] = [str(args.bench_directory.resolve()),
                    str(args.repo.resolve() / "experiments/sound-hardware-v1/sj6-factory")]
    from factory import canonical_bytes, load_factory, select
    from macho_dsp import DSP
    from unicorn.arm64_const import UC_ARM64_REG_X2

    record = select(load_factory(args.banks), BANK_UID, UID)["preset"]
    if sha256(canonical_bytes(record)).hexdigest() != RECORD_SHA:
        raise ValueError("Wrong authored Medieval Sky record")
    controls = {row["identifier"]: row["value"] for row in record["parameterValues"]}
    switches = [controls[name] for name in ("neonchorusIEnable", "neonchorusIIEnable")]
    if switches != [0, 0]:
        raise ValueError("The authenticated factory record does not request bypass")

    chorus = DSP()
    if sha256(chorus.image.b).hexdigest() != EXE_SHA:
        raise ValueError("Wrong original executable")
    chorus.allowed.extend([(0x100004C28, 0x100004C34), (0x100004C34, 0x100004C3C)])
    chorus.sd(0, float(SAMPLE_RATE))
    effect, _ = chorus.call(0x100004000, x0=2, seconds=6., instructions=4000000)
    payloads = [b"".join(dry[i + chan * 4:i + chan * 4 + 4] for i in range(0, len(dry), 8))
                for chan in range(2)]
    sources = [guarded(chorus, row) for row in payloads]
    outputs = [guarded(chorus, bytes(FRAMES * 4)) for _ in range(2)]

    def abl(buffers):
        address = chorus.alloc(40)
        chorus.wr(address, struct.pack("<I4xIIQIIQ", 2, 1, FRAMES * 4, buffers[0],
                                     1, FRAMES * 4, buffers[1]))
        return address

    endpoints = abl(sources), abl(outputs)
    chorus.uc.reg_write(UC_ARM64_REG_X2, endpoints[1])
    chorus.call(0x100004C14, x0=effect, x1=endpoints[0])

    def process(active):
        chorus.call(0x100004638, x0=effect)
        for parameter, value in enumerate((1., .85, 1.1, 1.)):
            chorus.sf(0, value)
            chorus.uc.reg_write(UC_ARM64_REG_X2, 1)
            chorus.call(0x10000422C, x0=effect, x1=parameter)
        chorus.call(0x100004C28 if active else 0x100004C34, x0=effect)
        for address in outputs:
            chorus.wr(address, b"\xa5" * (FRAMES * 4))
        for offset in range(0, FRAMES, 64):
            chorus.uc.reg_write(UC_ARM64_REG_X2, offset)
            chorus.call(0x100004730, x0=effect, x1=64, seconds=2., instructions=2000000)
        for address in sources + outputs:
            if chorus.rd(address - 32, 32) != b"G" * 32 or chorus.rd(address + FRAMES * 4, 32) != b"G" * 32:
                raise ValueError("Chorus touched a guard")
        if [chorus.rd(address, FRAMES * 4) for address in sources] != payloads:
            raise ValueError("Chorus changed an input buffer")
        channels = [chorus.rd(address, FRAMES * 4) for address in outputs]
        return b"".join(channels[0][i:i + 4] + channels[1][i:i + 4] for i in range(0, FRAMES * 4, 4))

    bypass, wet = process(False), process(True)
    if bypass != dry or wet == dry or not all(math.isfinite(x[0]) for x in struct.iter_unpack("<f", wet)):
        raise ValueError("Original chorus bypass or wet control failed")
    print(json.dumps({"matrix_sha256": MATRIX_SHA, "sampler_recorded_pcm_sha256": original_sha,
                      "sampler_source_float32_sha256": plan["source_float32_sha256"],
                      "authored_preset_uid": UID, "factory_chorus_switches": switches,
                      "sample_rate": SAMPLE_RATE, "frames": FRAMES,
                      "bypass_bit_exact": True, "bypass_pcm_sha256": sha256(bypass).hexdigest(),
                      "diagnostic_wet_pcm_sha256": sha256(wet).hexdigest(),
                      "wet_settings": {"mode": 1, "rate": .85, "depth": 1.1, "mix": 1.},
                      "wet_settings_are_authored": False, "factory_preset_applied": False,
                      "complete_graph_instances": 0, "original_stereo_captures": 0,
                      "qualified_s4_pairs": 0}, indent=2))


if __name__ == "__main__":
    main()
