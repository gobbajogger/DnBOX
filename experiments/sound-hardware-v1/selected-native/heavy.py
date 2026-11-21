#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Replay the unshifted calibrated scene on a prepared, populated project."""
import argparse
import json
import struct
from pathlib import Path
import sys

from run import measure

TIMING = Path(__file__).resolve().parent.parent / "strict-timing"
sys.path.insert(0, str(TIMING))
import replay


# firmware/hal/fm1_irq.h keeps the last fault in .noinit across guest resets.
# The handler flag is normal BSS and stays set while core zero reports a fault.
def fault_snapshot(memory, symbols):
    def words(name, count):
        offset = symbols[name] - 0x01c00000
        if offset < 0 or offset + count * 4 > len(memory):
            raise ValueError(f"{name} is outside the captured guest RAM")
        return struct.unpack_from(f"<{count}I", memory, offset)

    record = words("fm1_crash", 17)
    active, = words("fm1_fault_c.in_fault", 1)
    if record[0] != 0x43525348:
        return {"record": None, "handler_active": bool(active)}
    fields = ("magic", "count", "vector", "pc", "return_pc", "emulator_status",
              "debug_status", "stack_pointer", "psr", "icfg", "uptime_ms",
              "etm0", "etm1", "etm2", "etm3", "early", "core")
    return {"record": dict(zip(fields, record)), "handler_active": bool(active)}


def capture_health(boot, shot, boot_fault, shot_fault):
    # Both captures start from the same seeded flash and carry the same input
    # prefix. Subtract the measured startup silence rather than assuming when
    # a particular image enables its DAC. The DMA emits 512 stereo s16 frames.
    elapsed_ms = shot["duration_ms"] - boot["duration_ms"]
    expected_bytes = round(elapsed_ms * 44100 * 4 / 1000)
    captured_bytes = shot["pcm_bytes"] - boot["pcm_bytes"]
    dma_half_bytes = 512 * 4
    duration_ok = (elapsed_ms > 0 and boot["pcm_bytes"] > 0 and
                   boot["pcm_bytes"] % 4 == shot["pcm_bytes"] % 4 == 0 and
                   abs(captured_bytes - expected_bytes) <= dma_half_bytes)
    changed = boot_fault["record"] != shot_fault["record"]
    active = boot_fault["handler_active"] or shot_fault["handler_active"]
    retained = (boot_fault["record"] is not None and not changed and not active)
    return {"expected_post_boot_pcm_bytes": expected_bytes,
            "captured_post_boot_pcm_bytes": captured_bytes,
            "tolerance_pcm_bytes": dma_half_bytes, "duration_ok": duration_ok,
            "boot_fault": boot_fault, "shot_fault": shot_fault,
            "fault_record_changed": changed, "retained_fault": retained,
            "fault_ok": not changed and not active,
            "passed": duration_ok and not changed and not active}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("emulator", "elf", "fwsc", "seed", "output"):
        parser.add_argument("--" + name, type=lambda p: Path(p).resolve(), required=True)
    parser.add_argument("--owners", type=int, nargs=2, required=True, choices=range(4))
    args = parser.parse_args()
    scene = TIMING / "heavy.keys"
    if replay.digest(scene) != "d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271":
        parser.error("Input differs from the calibrated heavy scene")
    events = []
    for event in scene.read_text().strip().split(","):
        timestamp, key = event.split(":", 1)
        events.append((int(timestamp), key))
    args.output.mkdir(parents=True)
    symbols = replay.read_symbols(args.elf)
    boot = replay.run(args, symbols, events, "boot", 2000, 0, args.seed)
    shot = replay.run(args, symbols, events, "heavy", 8000, 0, args.seed)
    guest = measure(args.elf, args.output / "heavy", duration_ms=8000)
    owners_ok = guest["bass_kinds"] == args.owners and guest["runtime_status"] == [1, 1]
    all_active = (all(guest["peak_meter_909_808_A_B_BREAK"])
                  and guest["triggered_rows_909_808"] == [11, 11]
                  and guest["user_zones"] == [0, 1, 0]
                  and guest["loop_count"] == 1
                  and guest["first_catalog_slot_zone"] == [1, 0]
                  and guest["last_half_second_nonzero_words"] > 0)
    scene_late = shot["raw_late"] - boot["raw_late"]
    timing_ok = (shot["max_us"] < 11609 and shot["raw_late"] == 0
                 and scene_late == 0 and shot["audio_xruns"] == 0)
    faults = [fault_snapshot((args.output / name / "ram.bin").read_bytes(), symbols)
              for name in ("boot", "heavy")]
    health = capture_health(boot, shot, *faults)
    passed = owners_ok and all_active and timing_ok and health["passed"]
    result = {name + "_sha256": replay.digest(getattr(args, name))
              for name in ("elf", "fwsc", "emulator", "seed")}
    result.update(boot=boot, shot=shot, guest=guest, capture_health=health, scene_late=scene_late,
                  owners_ok=owners_ok, all_active=all_active, timing_ok=timing_ok, passed=passed)
    (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
    print(f"owners={guest['bass_kinds']} all_active={all_active} "
          f"max_us={shot['max_us']} raw_late={shot['raw_late']} "
          f"scene_late={scene_late} xruns={shot['audio_xruns']} capture_ok={health['passed']}")
    return int(not passed)


if __name__ == "__main__":
    raise SystemExit(main())
