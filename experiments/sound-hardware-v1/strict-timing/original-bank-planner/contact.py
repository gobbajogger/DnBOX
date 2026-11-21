#!/usr/bin/env python3
"""Capture the 909 command queue at the first Elite +1 ms audition discrepancy."""
import argparse
import json
from pathlib import Path
import struct
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("strict-dir", "emulator", "elf", "fwsc", "full-run", "output"):
        parser.add_argument("--" + name, required=True, type=lambda p: Path(p).resolve())
    args = parser.parse_args()
    sys.path.insert(0, str(args.strict_dir))
    import replay

    primary = json.loads((args.full_run / "sweep.json").read_bytes())
    for key in ("elf", "fwsc", "emulator"):
        if replay.digest(getattr(args, key)) != primary[key + "_sha256"]:
            raise ValueError(f"{key} differs from the primary replay")
    scene = args.strict_dir / "heavy.keys"
    events = [(int(t), payload) for t, payload in
              (event.split(":", 1) for event in scene.read_text().strip().split(","))]
    symbols = replay.read_symbols(args.elf)
    args.output.mkdir(parents=True)
    shot = replay.run(args, symbols, events, "window", 3181, 1, args.full_run / "seed/flash.bin")
    window = args.output / "window"
    if not (args.full_run / "shift1/stereo.s16le").read_bytes().startswith((window / "stereo.s16le").read_bytes()):
        raise ValueError("Short shot does not reproduce the primary physical output")
    if replay.digest(window / "scene.keys") != replay.digest(args.full_run / "shift1/scene.keys"):
        raise ValueError("Short shot used a different stimulus")
    ram = (window / "ram.bin").read_bytes()

    def field(symbol, offset, size):
        start = symbols[symbol] - 0x01c00000 + offset
        return ram[start:start + size]

    def word(symbol, offset=0):
        return struct.unpack("<I", field(symbol, offset, 4))[0]

    read, write = word("cq_r"), word("cq_w")
    if write - read not in range(1025):
        raise ValueError("Invalid command queue cursor distance")
    shot.update(raw_notes=word("fm1_in"), latch=word("ui", 112),
                last_hits=struct.unpack("<22I", field("eng_hit_ms", 0, 88)),
                hit_levels=list(field("eng_hit", 0, 22)), full_pcm_prefix_matches=True,
                command_queue=dict(read=read, write=write,
                                   pending=[f"0x{word('cq', 4*(n % 1024)):08x}" for n in range(read, write)],
                                   word_at_263=f"0x{word('cq', 4*263):08x}"))
    (args.output / "state.json").write_text(json.dumps(shot, indent=2) + "\n")


if __name__ == "__main__":
    main()
