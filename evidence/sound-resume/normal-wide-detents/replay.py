#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Isolate the two normal-WIDE panel detents on the archived Pro firmware."""
import argparse
import json
from pathlib import Path
import sys
from types import SimpleNamespace


PINS = {
    "emulator": "47587aba833096a6df95439657e0febd15ba40f4128576cbe9e40660a1eea8ab",
    "elf": "7a2282d54ac994bff45b344dce8274d9bb0922ebc1db717f2467097db08b20d9",
    "fwsc": "4f8ebbaa13fca3d357d131f7e101e6611b8aba42944da97ec2379a473feeec22",
    "seed": "2bb034c46221cc928cdc2757fcc0ca6bad007d3bbd046b0c9e61d7a53cdc0531",
    "scene": "d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271",
}
WAVE = (4560, "e4:+80")
WIDTH = (4565, "e6:+80")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source", "archive", "emulator", "output"):
        parser.add_argument(name, type=lambda value: Path(value).resolve())
    options = parser.parse_args()
    sys.path.insert(0, str(options.source / "experiments/sound-hardware-v1/strict-timing"))
    import replay

    archive = options.archive
    inputs = {
        "emulator": options.emulator,
        "elf": archive / "build/smk37/x0x.elf",
        "fwsc": archive / "build/x0x-smk37.fwsc",
        "seed": archive / "pro/seed/flash.bin",
        "scene": archive / "pro/bounded/heavy/scene.keys",
    }
    for name, path in inputs.items():
        if replay.digest(path) != PINS[name]:
            parser.error(f"SHA-256 mismatch for {name}")
    events = []
    for token in inputs["scene"].read_text().strip().split(","):
        timestamp, payload = token.split(":", 1)
        events.append((int(timestamp), payload))
    if events.count(WAVE) != 1 or events.count(WIDTH) != 1:
        parser.error("Expected one Wave detent and one Width detent")
    options.output.mkdir()
    args = SimpleNamespace(**inputs, output=options.output, public_recording=True)
    symbols = replay.read_symbols(args.elf)
    for name, omitted in (("without-wave", {WAVE}), ("without-width", {WIDTH}),
                          ("without-both", {WAVE, WIDTH})):
        selected = [event for event in events if event not in omitted]
        result = replay.run(args, symbols, selected, name, 8000, 0, args.seed)
        print(json.dumps({"case": name, **result}), flush=True)


if __name__ == "__main__":
    main()
