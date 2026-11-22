#!/usr/bin/env python3
"""Read actual WIDE DSP state using exact packaged ELF symbols and target member offsets."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def layout(path):
    return struct.unpack("<16I", path.read_bytes())

def symbols(elf):
    result = {}
    for line in subprocess.check_output(["nm", "-S", str(elf)], text=True).splitlines():
        parts = line.split()
        if len(parts) != 4:
            continue
        address, size, _, name = parts
        result[name] = (int(address, 16) - 0x01C00000, int(size, 16))
    return result

def score(ram_path, name_map, offsets, juno_index):
    memory = ram_path.read_bytes()
    stride, kind, status, filter_type, sub, prepared, voice_stride, voices, typed, stock, mode, follow, mix, phase, playing, seq_size = offsets
    start, size = name_map["original_bass"]
    sequencer, sequencer_size = name_map["seq"]
    assert size == 2 * stride and sequencer_size == seq_size
    assert 0 <= start < start + size <= len(memory)
    assert 0 <= sequencer < sequencer + sequencer_size <= len(memory)
    juno = start + juno_index * stride
    synth = start + (1 - juno_index) * stride
    indexes = (start, start + stride)
    return {
        "ram_sha256": sha(ram_path),
        "owner_kinds": [memory[index + kind] for index in indexes],
        "owner_status": [memory[index + status] for index in indexes],
        "juno_owner": "AB"[juno_index],
        "stock_wide": memory[juno + stock],
        "typed": memory[juno + typed],
        "mode": memory[juno + mode],
        "follow_panel": memory[juno + follow],
        "mix": struct.unpack_from("<f", memory, juno + mix)[0],
        "phase": struct.unpack_from("<I", memory, juno + phase)[0],
        "synthone_filter": struct.unpack_from("<f", memory, synth + filter_type)[0],
        "synthone_sub": struct.unpack_from("<f", memory, synth + sub)[0],
        "prepared_voice_filters": [memory[synth + prepared + i * voice_stride] for i in range(voices)],
        "sequencer_playing": memory[sequencer + playing],
        "worker_gave_up": memory[name_map["fm1_c1_gave_up"][0]],
    }

def qualify(boot, played, owners):
    return (
        all(row["typed"] == row["stock_wide"] == row["follow_panel"] == 1
            and row["mode"] == 2 and row["mix"] > 0
            and row["owner_kinds"] == owners and row["owner_status"] == [1, 1]
            and row["synthone_filter"] == 2.0 and row["synthone_sub"] == 1.0
            for row in (boot, played))
        and boot["phase"] != played["phase"] and not boot["sequencer_playing"]
        and played["sequencer_playing"] and 2 in played["prepared_voice_filters"]
        and not played["worker_gave_up"]
    )

def main():
    root = Path(sys.argv[1])
    offsets_path = root / "runtime-layout.bin"
    offsets = layout(offsets_path)
    status = 0
    for model, board, juno_index in [("elite", "smk37-elite", 0), ("pro", "smk37", 1)]:
        replay = root / model / "bounded"
        result_path = replay / "results.json"
        result = json.loads(result_path.read_text())
        elf = Path("build", board, "x0x.elf")
        fwsc = Path("build", "x0x-" + board + ".fwsc")
        seed = root / model / "seed" / "flash.bin"
        assert all(result[key + "_sha256"] == sha(input_file) for key, input_file in [("elf", elf), ("fwsc", fwsc), ("seed", seed)])
        labels = symbols(elf)
        boot = score(replay / "boot" / "ram.bin", labels, offsets, juno_index)
        played = score(replay / "heavy" / "ram.bin", labels, offsets, juno_index)
        owners = [3, 2] if juno_index == 0 else [2, 3]
        passed = qualify(boot, played, owners)
        report = {"source_sha": "1c46c6fc8d7a550b3308515a3415ea63d86e7486",
                  "result_sha256": sha(result_path), "layout_sha256": sha(offsets_path),
                  "runtime_passed": passed, "timing_passed": result["passed"],
                  "boot": boot, "heavy": played}
        (root / model / "runtime-readback.json").write_text(json.dumps(report, indent=2) + "\n")
        print(model, "runtime passed:", passed, played)
        status |= int(not passed or not result["passed"])
    return status

if __name__ == "__main__":
    raise SystemExit(main())
