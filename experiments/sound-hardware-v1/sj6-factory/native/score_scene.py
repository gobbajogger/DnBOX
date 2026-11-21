#!/usr/bin/env python3
"""Compare one strict panel capture with the same model and shift of stock."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import tarfile

TIMING = Path(__file__).resolve().parents[2] / "strict-timing"
sys.path.insert(0, str(TIMING))
from replay import digest, read_symbols
sys.path.insert(0, str(TIMING.parents[2] / "tools"))
from fm1_rescue import package_flash_image


def score(model, elf, folder, offset, stock_seed):
    board = "smk37-elite" if model == "elite" else "smk37"
    package = elf.parents[1] / f"x0x-{board}.fwsc"
    protected = package_flash_image(package.read_bytes())
    candidate_seed = folder.parent / "seed" / "flash.bin"

    series = json.loads((TIMING / "measurements.json").read_bytes())
    stock = next(row for row in series if row["series"] == "stock"
                 and row["model"] == model and row["shot_ms"] == 8000
                 and row["shift_ms"] == offset)
    with tarfile.open(TIMING / "direct-dma-startup.tar.gz") as archive:
        member = f"direct-dma-startup/exact-size/{model}/physical-runs.json"
        paired = next(row for row in json.load(archive.extractfile(member))
                      if row["project"]["shift_ms"] == offset)
    result = json.loads((folder / "result.json").read_bytes())
    pcm = folder / "stereo.s16le"
    if (result["duration_ms"], result["strict"], result["mhz"], result["events"],
            result["shift_ms"]) != (8000, 1, 360, 922, offset):
        raise ValueError("Scene parameters differ")
    if result["scene_sha256"] != stock["scene_sha256"]:
        raise ValueError("Stimulus differs from calibrated stock")
    if digest(stock_seed) != stock["initial_flash_sha256"]:
        raise ValueError("Comparison stock seed differs from the archived stock image")
    if digest(candidate_seed) != result["initial_flash_sha256"]:
        raise ValueError("Shot did not start with its own verified candidate seed")
    seed = candidate_seed.read_bytes()
    final_flash = (folder / "flash.bin").read_bytes()
    stock_flash = stock_seed.read_bytes()
    if len(seed) != 0x100000 or len(stock_flash) != len(seed) or len(final_flash) != len(seed):
        raise ValueError("Flash size differs from the target")
    if len(protected) != 0x9d000 or seed[:0x9d000] != protected or final_flash[:0x9d000] != protected:
        raise ValueError("Firmware bytes in the seed or final flash differ from the supplied package")
    if seed[0x9d000:] != stock_flash[0x9d000:]:
        raise ValueError("Initial user area differs from the same-model stock reference")
    if digest(folder / "flash.bin") != result["final_flash_sha256"]:
        raise ValueError("Final flash differs from the replay receipt")
    if digest(pcm) != result["pcm_sha256"] or pcm.stat().st_size != result["pcm_bytes"]:
        raise ValueError("Captured PCM differs from the replay receipt")
    symbols = read_symbols(elf)
    ram = (folder / "ram.bin").read_bytes()
    start = symbols["proj"] - 0x01c00000
    project = ram[start:start + 36420]
    if len(project) != 36420 or project[:4] != struct.pack("<I", 0x50305830):
        raise ValueError("Saved project size or magic differs")
    project_sha = hashlib.sha256(project).hexdigest()
    if (paired["physical"]["stock_sha256"] != stock["pcm_sha256"]
            or paired["project"]["stock_elf_sha256"] != stock["elf_sha256"]):
        raise ValueError("Archived stock PCM or ELF identities disagree")
    return dict(model=model, **result, candidate_elf_sha256=digest(elf),
                candidate_package_sha256=digest(package),
                protected_flash_sha256=hashlib.sha256(protected).hexdigest(),
                initial_user_area_sha256=hashlib.sha256(seed[0x9d000:]).hexdigest(),
                stock_max_us=stock["debug"]["max_us"],
                stock_pcm_sha256=stock["pcm_sha256"], project_sha256=project_sha,
                stock_project_sha256=paired["project"]["stock_project_sha256"],
                pcm_matches_stock=result["pcm_sha256"] == stock["pcm_sha256"],
                project_matches_stock=project_sha == paired["project"]["stock_project_sha256"],
                margin_us=11609 - result["max_us"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("model", choices=("elite", "pro"))
    parser.add_argument("elf", type=Path)
    parser.add_argument("folder", type=Path)
    parser.add_argument("offset", type=int, choices=range(6))
    parser.add_argument("--stock-seed", type=Path, required=True)
    args = parser.parse_args()
    result = score(args.model, args.elf, args.folder, args.offset, args.stock_seed)
    print(json.dumps(result, indent=2))
    sys.exit(0 if result["pcm_matches_stock"] and result["project_matches_stock"] else 1)
