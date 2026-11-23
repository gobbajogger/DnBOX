#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Replay the unshifted calibrated scene on a prepared, populated project."""
import argparse
import json
from pathlib import Path
import sys

from run import measure

TIMING = Path(__file__).resolve().parent.parent / "strict-timing"
sys.path.insert(0, str(TIMING))
import replay


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
    passed = owners_ok and all_active and timing_ok
    result = {name + "_sha256": replay.digest(getattr(args, name))
              for name in ("elf", "fwsc", "emulator", "seed")}
    result.update(boot=boot, shot=shot, guest=guest, scene_late=scene_late,
                  owners_ok=owners_ok, all_active=all_active, timing_ok=timing_ok, passed=passed)
    (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
    print(f"owners={guest['bass_kinds']} all_active={all_active} "
          f"max_us={shot['max_us']} raw_late={shot['raw_late']} "
          f"scene_late={scene_late} xruns={shot['audio_xruns']}")
    return int(not passed)


if __name__ == "__main__":
    raise SystemExit(main())
