#!/usr/bin/env python3
"""Verify the native stock firmware's DNR2 flow over the emulator-only USB bus."""
from argparse import ArgumentParser
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def pack(value, n):
    return bytes((value >> (i * 7)) & 127 for i in range(n))


def unpack(frame, start, n):
    return sum(v << (i * 7) for i, v in enumerate(frame[start:start + n]))


def command(op, request, unit=0, high=(0, 0, 0), epoch=0, revision=0, control=0, value=0):
    return (bytes([0xf0, 0x7d, 0x44, 0x4e, 2, op]) + pack(unit, 4) + pack(42, 4)
            + pack(epoch, 4) + pack(request, 4) + pack(revision, 4)
            + pack(control, 2) + pack(0, 2) + pack(value, 5)
            + b"".join(pack(word, 5) for word in high) + b"\xf7")


def run(args, name, commands):
    run_dir = args.output / name
    run_dir.mkdir(parents=True)
    stream = ";".join(f"{ms}:{message.hex()}" for ms, message in commands)
    env = {key: val for key, val in os.environ.items() if not key.startswith("FM1_")}
    env.update(FM1_STRICT="1", FM1_NO_SKIP="1", FM1_USB_SEND=stream)
    invocation = [str(args.emulator), "--fwsc", str(args.fwsc), "--mhz", "360",
                  "--shot", "2600", "--flash", str(run_dir / "flash.bin"), "--usb"]
    log = run_dir / "usb.log"
    with log.open("w") as output:
        subprocess.run(invocation, cwd=run_dir, env=env, stdout=output,
                       stderr=subprocess.STDOUT, check=True)
    lines = log.read_text()
    assert "skipped 0 cycles" in lines
    frames = [bytes.fromhex(text) for text in re.findall(r"usb in:  ([0-9a-f ]+)\s*\n", lines)]
    return [frame for frame in frames if frame[:5] == b"\xf0\x7d\x44\x4e\x02"]


def decode(frame):
    assert len(frame) == 51 and frame[-1] == 0xf7 and (frame[5] & 64)
    return dict(op=frame[5] & 63, status=frame[6], kind=frame[7],
                unit=unpack(frame, 8, 4), high=tuple(unpack(frame, 35 + i * 5, 5) for i in range(3)),
                client=unpack(frame, 12, 4), epoch=unpack(frame, 16, 4),
                request=unpack(frame, 20, 4), revision=unpack(frame, 24, 4),
                control=unpack(frame, 28, 2), value=unpack(frame, 30, 5))


def main():
    cli = ArgumentParser(description=__doc__)
    for name in ("emulator", "fwsc", "output"):
        cli.add_argument("--" + name, required=True, type=lambda arg: Path(arg).resolve())
    args = cli.parse_args()
    identity, = map(decode, run(args, "discover", [(1800, command(0, 1))]))
    assert identity["status"] == 0 and identity["revision"] == 1
    assert identity["client"] == 42 and identity["request"] == 1
    assert identity["control"] == 1 and identity["value"] == 0xecca5979
    assert identity["unit"] and any(identity["high"])
    binding = dict(unit=identity["unit"], high=identity["high"])

    def addressed(op, number, **fields):
        return command(op, number, **binding, **fields)

    # Pinned public emulator resets its deterministic RNG for each fresh run.
    messages = [(1800, command(0, 1)), (1830, addressed(1, 2)),
                (1860, addressed(2, 3, epoch=1, revision=1, control=1)),
                (1890, addressed(3, 4, epoch=1, revision=1, control=1, value=76)),
                (1920, addressed(2, 5, epoch=1, revision=2, control=1)),
                (1950, addressed(3, 6, epoch=1, revision=2, control=1, value=75)),
                (1980, addressed(2, 7, epoch=1, revision=3, control=1)),
                (2010, addressed(7, 8, epoch=1, revision=3)),
                (2040, addressed(3, 9, epoch=1, revision=3, control=1, value=100))]
    replies = list(map(decode, run(args, "exchange", messages)))
    assert len(replies) == 9 and replies[0] == identity
    for frame, (op, request, status, revision) in zip(replies, [
        (0, 1, 0, 1), (1, 2, 0, 1), (2, 3, 0, 1), (3, 4, 1, 2),
        (2, 5, 0, 2), (3, 6, 1, 3), (2, 7, 0, 3), (7, 8, 0, 3), (3, 9, 4, 3),
    ]):
        assert (frame["op"], frame["request"], frame["status"], frame["revision"]) == (op, request, status, revision)
        assert frame["unit"] == binding["unit"] and frame["high"] == binding["high"] and frame["client"] == 42
    assert [frame["value"] for frame in replies[2:7]] == [75, 76, 76, 75, 75]
    assert all(frame["epoch"] == 1 for frame in replies[1:])

    receipt = {name + "_sha256": hashlib.sha256(getattr(args, name).read_bytes()).hexdigest()
               for name in ("emulator", "fwsc")}
    receipt.update(software_only=True, transport="emulated USB MIDI",
                   fresh_deterministic_boot=True, replies=replies)
    (args.output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print("Native emulated USB: catalog, addressed Apply/Undo, CLOSE and stale-write refusal PASS")


if __name__ == "__main__":
    main()
