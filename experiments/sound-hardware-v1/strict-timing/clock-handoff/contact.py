"""Read the key scanner, UI key latch and 909 track-4 hit from an Elite +1 ms replay."""
import argparse
import json
from pathlib import Path
import sys

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("packet", "emulator", "elf", "fwsc", "full-run", "output"):
        parser.add_argument("--" + name, required=True, type=lambda s: Path(s).resolve())
    args = parser.parse_args()
    sys.path.insert(0, str(args.packet))
    import replay

    full = json.loads((args.full_run / "sweep.json").read_text())
    assert replay.digest(args.elf) == full["elf_sha256"]
    assert replay.digest(args.fwsc) == full["fwsc_sha256"]
    assert replay.digest(args.emulator) == full["emulator_sha256"]
    tokens = (args.packet / "heavy.keys").read_text().strip().split(",")
    events = []
    for token in tokens:
        timestamp, event = token.split(":", 1)
        events.append((int(timestamp), event))
    args.output.mkdir(parents=True, exist_ok=False)
    syms = replay.read_symbols(args.elf)
    sample = replay.run(args, syms, events, "sample", 2650, 1, args.full_run / "seed/flash.bin")
    run = args.output / "sample"
    original = args.full_run / "shift1"
    assert replay.digest(run / "scene.keys") == replay.digest(original / "scene.keys")
    assert (original / "stereo.s16le").read_bytes().startswith((run / "stereo.s16le").read_bytes())
    ram = (run / "ram.bin").read_bytes()

    def field(symbol, byte_offset=0, size=4):
        pos = syms[symbol] - 0x01c00000 + byte_offset
        return int.from_bytes(ram[pos:pos + size], "little")

    result = dict(shot=sample, elf=full["elf_sha256"], native_raw_notes=field("fm1_in"),
                  ui_key_latch=field("ui", 112), drum4_velocity=field("eng_hit", 4, 1),
                  drum4_last_hit_ms=field("eng_hit_ms", 16), physical_pcm_prefix_matches_full_shot=True)
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()
