#!/usr/bin/env python3
"""Compare stock and opt-in runs from the same source, model and physical stimulus."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / 'sj6-factory/native'))
sys.path.insert(0, str(HERE.parents[2] / 'tools'))
from score_scene import compare_raw, digest, read_project
from fm1_rescue import package_flash_image


def captures(directory, elf, fwsc):
    report = json.loads((directory / 'sweep.json').read_bytes())
    assert report['elf_sha256'] == digest(elf)
    assert report['fwsc_sha256'] == digest(fwsc)
    seed = directory / 'seed/flash.bin'
    protected = package_flash_image(fwsc.read_bytes())
    seed_bytes = seed.read_bytes()
    assert len(protected) == 0x9d000 and len(seed_bytes) == 0x100000
    assert seed_bytes[:len(protected)] == protected
    shots = {}
    for row in report['runs']:
        shift = row['shift_ms']
        folder = directory / f'shift{shift}'
        assert shift not in shots and 0 <= shift <= 5
        assert (row['duration_ms'], row['strict'], row['mhz'], row['events']) == (8000, 1, 360, 922)
        assert row['initial_flash_sha256'] == digest(seed)
        assert row['scene_sha256'] == digest(folder / 'scene.keys')
        assert row['final_flash_sha256'] == digest(folder / 'flash.bin')
        flashed = (folder / 'flash.bin').read_bytes()
        assert len(flashed) == len(seed_bytes) and flashed[:len(protected)] == protected
        assert row['pcm_sha256'] == digest(folder / 'stereo.s16le')
        assert row['pcm_bytes'] == (folder / 'stereo.s16le').stat().st_size
        shots[shift] = row
    assert set(shots) == set(range(6))
    return report, shots, seed_bytes[len(protected):]


def main():
    if not __debug__:
        raise RuntimeError("Run with Python assertions enabled to verify the input receipts")
    parser = argparse.ArgumentParser(description=__doc__)
    for profile in ('stock', 'wide'):
        for path in ('dir', 'elf', 'fwsc'):
            parser.add_argument(f'--{profile}-{path}', required=True, type=Path)
    args = parser.parse_args()
    stock, controls, control_state = captures(args.stock_dir, args.stock_elf, args.stock_fwsc)
    wide, variants, variant_state = captures(args.wide_dir, args.wide_elf, args.wide_fwsc)
    assert stock['emulator_sha256'] == wide['emulator_sha256']
    assert control_state == variant_state
    rows = []
    for shift in sorted(controls):
        control, variant = controls[shift], variants[shift]
        assert control['scene_sha256'] == variant['scene_sha256']
        base = args.stock_dir / f'shift{shift}'
        test = args.wide_dir / f'shift{shift}'
        project = read_project(args.wide_elf, test)
        expected = hashlib.sha256(read_project(args.stock_elf, base)).hexdigest()
        differences = compare_raw(base, args.stock_elf, {**control, 'elf_sha256': stock['elf_sha256']},
                                  expected, (test / 'stereo.s16le').read_bytes(), project)
        rows.append(dict(shift_ms=shift, stock=control, wide=variant, differences=differences,
                         stock_project_sha256=expected, wide_project_sha256=hashlib.sha256(project).hexdigest()))
    keys = ('elf_sha256', 'fwsc_sha256', 'emulator_sha256', 'boot')
    print(json.dumps(dict(stock={k: stock[k] for k in keys}, wide={k: wide[k] for k in keys},
                          initial_user_area_sha256=hashlib.sha256(control_state).hexdigest(), shots=rows), indent=2))


if __name__ == '__main__':
    main()
