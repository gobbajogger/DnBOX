# SPDX-License-Identifier: GPL-3.0-only
"""Play a two-synth project with the original 909FL loop on an isolated SMK emulator."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import wave
import zlib

import numpy as np

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / "tools"))
from fm1_rescue import package_flash_image
from sampleio import SLOT_DATA_OFF, user_slot


def sha(data):
    return hashlib.sha256(data).hexdigest()


def install_project(flash, objects):
    # firmware/src/storage.c: one complete commit to the A copy of each saved object.
    pos = 0
    for obj in range(7):
        size, = struct.unpack_from("<I", objects, pos)
        pos += 4
        payload = objects[pos:pos + size]
        pos += size
        if not size:
            continue
        if size > 3840 or len(payload) != size:
            raise ValueError("Invalid project_save object")
        start = 0x9d000 + obj * 8192
        flash[start:start + 8192] = b"\xff" * 8192
        header = struct.pack("<IHH5I", 0x30583058, obj, 0, 1, size,
                             zlib.crc32(payload), 0xffffffff, 0xffffffff)
        flash[start:start + 32] = header + struct.pack("<I", zlib.crc32(header))
        flash[start + 256:start + 256 + size] = payload
    if pos != len(objects):
        raise ValueError("Invalid project object extent")


def prepare_flash(args, objects):
    package = package_flash_image(args.fwsc.read_bytes())
    stock = args.stock_seed.read_bytes()
    if len(package) != 0x9d000 or len(stock) != 0x100000:
        raise ValueError("Expected a 1 MiB SMK seed and its protected package extent")
    if stock[0xaf000:0xaf00c] != b"SMK37DATAX0X":
        raise ValueError("Use a warmed X0X SMK seed")
    flash = bytearray(stock)
    flash[:len(package)] = package
    install_project(flash, objects)

    # Slot two has the 120 KiB needed for this full-rate loop.
    sample = REPO / "assets/factory-breaks/909FL.wav"
    with wave.open(str(sample)) as wav:
        if (wav.getsampwidth(), wav.getnchannels(), wav.getframerate()) != (2, 1, 44100):
            raise ValueError("Unexpected sample format")
        samples = np.frombuffer(wav.readframes(wav.getnframes()), "<i2")
    header, data = user_slot("909FL", [(samples.tolist(), 60, 0, 127)], 44100, 0x1e000)
    if data != (sample.parent / "909FL.adpcm").read_bytes():
        raise ValueError("User-slot PCM conversion differs from the checked-in ADPCM")
    slot = 0xd6000
    flash[slot:slot + len(header)] = header
    flash[slot + SLOT_DATA_OFF:slot + SLOT_DATA_OFF + len(data)] = data
    provenance = {
        "emulator_sha256": sha(args.emulator.read_bytes()),
        "firmware_sha256": sha(args.fwsc.read_bytes()),
        "elf_sha256": sha(args.elf.read_bytes()),
        "stock_seed_sha256": sha(stock),
        "project_objects_sha256": sha(objects),
        "source_sample_sha256": sha(sample.read_bytes()),
        "slot_adpcm_sha256": sha(data),
        "initial_flash_sha256": sha(flash),
    }
    return flash, provenance


def run_shot(args, out, flash):
    flash_path = out / "flash.bin"
    flash_path.write_bytes(flash)
    keys = out / "scene.keys"
    keys.write_text("1800:SEQ PLAY,1900:SEQ PLAY^\n")
    env = {k: v for k, v in os.environ.items() if not k.startswith("FM1_")}
    env.update(FM1_STRICT="1", FM1_BENCH_KEYS="@" + str(keys), FM1_DUMPRAM=str(out / "ram.bin"),
               FM1_DNBOX_PCM=str(out / "stereo.s16le"))
    with (out / "run.log").open("w") as log:
        subprocess.run([args.emulator.resolve(), "--fwsc", args.fwsc.resolve(), "--mhz", "360",
                        "--shot", "2800", "--flash", flash_path],
                       cwd=out, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)


def measure(elf, out, duration_ms=2800):
    log = (out / "run.log").read_text()
    if f"panel.ppm at {duration_ms} ms" not in log or "skipped 0 cycles" not in log:
        raise ValueError("Incomplete strict emulator shot")
    memory = (out / "ram.bin").read_bytes()
    symbols = {}
    for line in subprocess.check_output(["nm", "-S", elf], text=True).splitlines():
        fields = line.split()
        if len(fields) == 4:
            symbols[fields[-1]] = int(fields[0], 16) - 0x01c00000, int(fields[1], 16)

    def region(name):
        start, size = symbols[name]
        if start < 0 or start + size > len(memory):
            raise ValueError(f"{name} is outside guest RAM")
        return memory[start:start + size]

    # original_runtime.h: the 32-bit target's final word contains kind, status and padding.
    arenas = region("original_bass")
    stride = len(arenas) // 2
    kinds, status = zip(*(struct.unpack_from("<Bb", arenas, end - 4) for end in (stride, 2 * stride)))
    dbg = struct.unpack("<13I", region("x0x_dbg"))
    last_hit_ms = struct.unpack("<22I", region("eng_hit_ms"))
    triggered = [sum(t != 0 for t in last_hit_ms[:11]), sum(t != 0 for t in last_hit_ms[11:])]
    pcm = (out / "stereo.s16le").read_bytes()
    audio = np.frombuffer(pcm, "<i2").astype(np.int32)
    if len(audio) <= 2 * 44100 or len(audio) % 2:
        raise ValueError("Incomplete stereo DMA capture")
    return {
        "bass_kinds": list(kinds),
        "runtime_status": list(status),
        "peak_meter_909_808_A_B_BREAK": list(struct.unpack("<5H", region("eng_peak"))),
        "triggered_rows_909_808": triggered,
        "last_playhead_steps": list(region("eng_step")),
        "user_zones": list(region("usr_nz")),
        "loop_count": struct.unpack("<I", region("nloops"))[0],
        "first_catalog_slot_zone": [region("loop_slot")[0], region("loop_zone")[0]],
        "sample_rate": 44100,
        "stereo_frames": len(audio) // 2,
        "first_half_second_nonzero_words": int(np.count_nonzero(audio[:44100])),
        "last_half_second_nonzero_words": int(np.count_nonzero(audio[-44100:])),
        "peak_pcm16": int(np.abs(audio).max()),
        "pcm_sha256": sha(pcm),
        "observed_halves": dbg[1],
        "peak_callback_us": dbg[2],
        "raw_late": dbg[5],
        "audio_xruns": struct.unpack("<I", region("audio_xruns"))[0],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for arg in ("emulator", "elf", "fwsc", "stock_seed", "project_builder", "output"):
        parser.add_argument(arg, type=Path)
    parser.add_argument("bass_a", type=int, choices=range(4))
    parser.add_argument("bass_b", type=int, choices=range(4))
    parts = parser.add_mutually_exclusive_group()
    parts.add_argument("--kit", choices=("909", "808"), help="play only the selected kit")
    parts.add_argument("--ensemble", action="store_true", help="play both kits, both basses and BREAK")
    action = parser.add_mutually_exclusive_group()
    action.add_argument("--stage-only", action="store_true", help="emit a fresh seed for another scene")
    action.add_argument("--rescore", action="store_true", help="read a completed shot in OUTPUT")
    args = parser.parse_args()
    kit = 2 if args.ensemble else {None: -1, "909": 0, "808": 1}[args.kit]
    out = args.output.resolve()
    objects = out / "save-objects.bin"
    with tempfile.TemporaryDirectory() as tmp:
        rebuilt = Path(tmp) / "save-objects.bin"
        subprocess.run([args.project_builder.resolve(), str(args.bass_a), str(args.bass_b),
                        str(kit), rebuilt], check=True)
        project = rebuilt.read_bytes()
    if args.rescore:
        if objects.read_bytes() != project:
            raise ValueError("Captured project differs from the requested instrument pattern")
    else:
        (out / "work").mkdir(parents=True, exist_ok=False)
        objects.write_bytes(project)

    flash, provenance = prepare_flash(args, project)
    if args.stage_only:
        (out / "flash.bin").write_bytes(flash)
        (out / "inputs.json").write_text(json.dumps(provenance, indent=2) + "\n")
        return 0
    if not args.rescore:
        run_shot(args, out, flash)
    if (out / "flash.bin").read_bytes() != flash:
        raise ValueError("Staged flash does not match the scored package, sample and project")

    result = measure(args.elf, out)
    result.update(provenance)
    peaks = result["peak_meter_909_808_A_B_BREAK"]
    active = {-1: [2, 3, 4], 0: [0], 1: [1], 2: [0, 1, 2, 3, 4]}[kit]
    result["active_parts"] = active
    expected_hits = [11 if args.ensemble or kit == i else 0 for i in range(2)]
    passed = (result["bass_kinds"] == [args.bass_a, args.bass_b]
              and result["runtime_status"] == [1, 1]
              and [i for i, peak in enumerate(peaks) if peak] == active
              and result["triggered_rows_909_808"] == expected_hits
              and result["user_zones"] == [0, 1, 0]
              and result["loop_count"] == 1 and result["first_catalog_slot_zone"] == [1, 0]
              and result["first_half_second_nonzero_words"] == 0
              and result["last_half_second_nonzero_words"] > 0
              and result["peak_callback_us"] < 11609 and not result["raw_late"]
              and not result["audio_xruns"])
    result["passed"] = passed
    (out / "results.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))
    return int(not passed)


if __name__ == "__main__":
    raise SystemExit(main())
