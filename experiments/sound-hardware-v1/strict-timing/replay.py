#!/usr/bin/env python3
"""Run the calibrated SMK scene with the supplied fm1-live and matched board ELF."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from fm1_rescue import package_flash_image


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_symbols(elf):
    rows = subprocess.check_output(["nm", "-S", str(elf)], text=True)
    names = {}
    for row in rows.splitlines():
        fields = row.split()
        if len(fields) in (3, 4):
            names[fields[-1]] = int(fields[0], 16)
    return names


def validate_seed(fwsc, seed):
    """A restored emulator flash replaces package code as well as user state."""
    protected = package_flash_image(fwsc.read_bytes())
    if len(protected) != 0x9d000 or seed.stat().st_size != 0x100000:
        raise ValueError("Unexpected SMK package or flash extent")
    with seed.open("rb") as source:
        if source.read(len(protected)) != protected:
            raise ValueError("Seed firmware bytes differ from the requested package")


def run(args, symbols, events, name, duration, offset, seed=None):
    if seed is not None:
        validate_seed(args.fwsc, seed)
    path = args.output / name
    path.mkdir()
    (path / "work").mkdir()
    keys = path / "scene.keys"
    scene = [f"{time + offset}:{event}" for time, event in events]
    keys.write_text(",".join(scene) + "\n")
    flash = path / "flash.bin"
    if seed is not None:
        shutil.copyfile(seed, flash)
    initial_hash = digest(flash) if seed is not None else None
    ram = path / "ram.bin"
    pcm = path / "stereo.s16le"
    env = {k: v for k, v in os.environ.items() if not k.startswith("FM1_")}
    env.update(FM1_STRICT="1", FM1_BENCH_KEYS="@" + str(keys), FM1_DUMPRAM=str(ram),
               FM1_DNBOX_PCM=str(pcm), FM1_CORE_LOAD_CSV=str(path / "core.csv"),
               FM1_CORE_LOAD=f"{symbols['_c1_start']:x}-{symbols['_c1_end']:x}")
    command = [str(args.emulator), "--fwsc", str(args.fwsc), "--mhz", "360",
               "--shot", str(duration), "--flash", str(flash)]
    public_recording = getattr(args, "public_recording", False)
    with (path / "run.log").open("w") as log:
        if public_recording:
            # The public recorder streams PPM to stdout and writes boot-padded
            # DAC PCM in work/audio.raw. Keep its diagnostics on stderr.
            env["FM1_NO_SKIP"] = "1"
            subprocess.run([*command, "--record", "-"], env=env, cwd=path,
                           stdout=subprocess.DEVNULL, stderr=log, check=True)
        else:
            subprocess.run(command, env=env, cwd=path, stdout=log,
                           stderr=subprocess.STDOUT, check=True)
    if public_recording:
        shutil.move(path / "work/audio.raw", pcm)
    log_text = (path / "run.log").read_text()
    if f"panel.ppm at {duration} ms" not in log_text or "skipped 0 cycles" not in log_text:
        raise ValueError(f"Incomplete strict emulator window in {path}")
    memory = ram.read_bytes()
    debug = struct.unpack_from("<13I", memory, symbols["x0x_dbg"] - 0x01c00000)
    if debug[0] != 0x44424731 or not debug[1] or not pcm.is_file() or not pcm.stat().st_size:
        raise ValueError(f"Missing debug or audio capture in {path}")
    result = dict(duration_ms=duration, shift_ms=offset, events=len(events), strict=1, mhz=360,
                  halves=debug[1], max_us=debug[2], raw_late=debug[5],
                  audio_xruns=struct.unpack_from("<I", memory, symbols["audio_xruns"] - 0x01c00000)[0],
                  pcm_sha256=digest(pcm), pcm_bytes=pcm.stat().st_size, scene_sha256=digest(keys),
                  pcm_capture="public boot-padded recorder" if public_recording else "raw DAC observer",
                  initial_flash_sha256=initial_hash, final_flash_sha256=digest(flash))
    (path / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("emulator", "elf", "fwsc", "output"):
        parser.add_argument("--" + key, type=lambda s: Path(s).resolve(), required=True)
    parser.add_argument("--scene", type=Path, default=Path(__file__).with_name("heavy.keys"))
    parser.add_argument("--offsets", type=int, nargs="+", default=list(range(6)))
    args = parser.parse_args()
    if any(i < 0 or i > 5 for i in args.offsets) or len(set(args.offsets)) != len(args.offsets):
        parser.error("Offsets must be distinct integers in 0..5")
    if digest(args.scene) != "d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271":
        parser.error("Input differs from the calibrated 922-event scene")
    args.output.mkdir(parents=True)
    symbols = read_symbols(args.elf)
    events = []
    for token in args.scene.read_text().strip().split(","):
        timestamp, payload = token.split(":", 1)
        events.append((int(timestamp), payload))
    result = {key + "_sha256": digest(getattr(args, key)) for key in ("emulator", "elf", "fwsc")}
    result["seed"] = run(args, symbols, events, "seed", 2000, 0)
    seed = args.output / "seed/flash.bin"
    boot = run(args, symbols, events, "boot", 2000, 0, seed)
    result.update(boot=boot, runs=[])
    for offset in args.offsets:
        row = run(args, symbols, events, f"shift{offset}", 8000, offset, seed)
        row["scene_late"] = row["raw_late"] - boot["raw_late"]
        row["limit_margin_us"] = 11609 - row["max_us"]
        row["stock_half_count_delta"] = row["halves"] - 648
        row["target_pass"] = (row["max_us"] <= 10909 and row["scene_late"] == 0
                              and row["raw_late"] == 0)
        result["runs"].append(row)
        (args.output / "sweep.json").write_text(json.dumps(result, indent=2) + "\n")
        print(offset, row["halves"], row["max_us"], row["raw_late"], row["scene_late"], flush=True)
    return int(any(not row["target_pass"] for row in result["runs"]))


if __name__ == "__main__":
    raise SystemExit(main())
