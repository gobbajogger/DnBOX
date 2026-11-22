#!/usr/bin/env python3
"""Read state from exact firmware ELF symbols and captured guest RAM."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

root = Path(sys.argv[1])
fields = struct.unpack("<13I", (root / "runtime-layout.bin").read_bytes())
stride, kind_at, status_at, filter_at, sub_at, voice_filter_at, started_at, voice_size, voice_count, typed_at, stock_at, mode_at, mix_at = fields
for model, board, synth_index in [("elite", "smk37-elite", 1), ("pro", "smk37", 0)]:
    input_path = root / f"{model}-inputs.json"
    receipt = json.loads(input_path.read_text())
    elf = Path(receipt["new_elf"])
    assert sha(elf) == receipt["new_elf_sha256"]
    rows = subprocess.check_output(["nm", "-S", str(elf)], text=True).splitlines()
    symbol = next(row.split() for row in rows if row.split()[-1] == "original_bass")
    base = int(symbol[0], 16) - 0x01C00000
    assert int(symbol[1], 16) == 2 * stride
    sn = base + synth_index * stride
    jn = base + (1 - synth_index) * stride
    result_path = root / model / "results.json"
    result = json.loads(result_path.read_text())
    assert result["elf_sha256"] == receipt["new_elf_sha256"]
    assert result["fwsc_sha256"] == receipt["new_fwsc_sha256"]
    assert result["seed_sha256"] == receipt["new_seed_sha256"]
    decoded = {}
    for stage in ["boot", "heavy"]:
        path = root / model / stage / "ram.bin"
        ram = path.read_bytes()
        assert 0 <= base < base + 2 * stride <= len(ram)
        assert ram[sn + kind_at] == 2 and ram[jn + kind_at] == 3
        assert ram[sn + status_at] == ram[jn + status_at] == 1
        voices = [{"prepared_filter_type": ram[sn + voice_filter_at + i * voice_size],
                   "started": bool(ram[sn + started_at + i * voice_size])}
                  for i in range(voice_count)]
        stage_values = {
            "ram_sha256": sha(path),
            "synthone_part": "AB"[synth_index],
            "synthone_filter_type": struct.unpack_from("<f", ram, sn + filter_at)[0],
            "synthone_sub_level": struct.unpack_from("<f", ram, sn + sub_at)[0],
            "synthone_voices": voices,
            "juno_part": "BA"[synth_index],
            "juno_typed": bool(ram[jn + typed_at]),
            "juno_stock_wide_preset": bool(ram[jn + stock_at]),
            "juno_factory_chorus_mode": ram[jn + mode_at],
            "juno_legacy_chorus_mix": struct.unpack_from("<f", ram, jn + mix_at)[0],
        }
        assert stage_values["synthone_filter_type"] == 2.0 and stage_values["synthone_sub_level"] == 1.0
        assert not stage_values["juno_typed"] and not stage_values["juno_stock_wide_preset"]
        assert not stage_values["juno_factory_chorus_mode"]
        if stage == "heavy":
            assert any(voice["prepared_filter_type"] == 2 for voice in voices)
        decoded[stage] = stage_values
    (root / f"{model}-runtime-readback.json").write_text(json.dumps({
        "packages_verified": True, "results_sha256": sha(result_path),
        "input_receipt_sha256": sha(input_path),
        "target_layout_sha256": sha(root / "runtime-layout.bin"),
        "stages": decoded
    }, indent=2) + "\n")
