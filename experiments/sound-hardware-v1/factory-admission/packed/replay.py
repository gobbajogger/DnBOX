#!/usr/bin/env python3
"""Render from the C bank selector and check every stereo byte against the original factory manifest."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('build', type=Path, help='Fresh output of run_synthone_factory.py --build-only')
    cli.add_argument('out', type=Path)
    args = cli.parse_args()
    work, base = args.out.resolve(), args.build.resolve()
    work.mkdir(exist_ok=False)
    source = base / 'commands/ORIGINAL-PRESET-COMMANDS.json'
    subprocess.run([sys.executable, HERE / 'generate.py', source, work / 's1_factory_bank.inc'], check=True)
    bank_obj = work / 'bank.o'
    subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', '-I', work,
                    '-c', HERE / 'bank.c', '-o', bank_obj], check=True)
    commands = json.loads((base / 'factory-build-commands.json').read_bytes())
    compile_cmd, = (cmd for cmd in commands if '-c' in cmd and cmd[cmd.index('-c')+1].endswith('/preset_audio.cpp'))
    obj = work / 'render.o'
    original_obj = compile_cmd[compile_cmd.index('-o')+1]
    compile_cmd[compile_cmd.index('-c')+1] = str(HERE / 'render.cpp')
    compile_cmd[compile_cmd.index('-o')+1] = str(obj)
    compile_cmd[compile_cmd.index('-MF')+1] = str(work / 'render.d')
    subprocess.run(compile_cmd, check=True)
    link = commands[-1]
    link[link.index(original_obj):link.index(original_obj)+1] = [str(obj), str(bank_obj)]
    exe = work / 'render'
    link[link.index('-o')+1] = str(exe)
    subprocess.run(link, check=True)
    fixture = base / 'factory-inputs'
    manifest = ROOT / 'docs/sound/v2-synthone-factory-pcm-manifest.json'
    references = {r['file']: r for r in json.loads(manifest.read_bytes())['files']}
    corpus = fixture / 'assets/factory_banks/synthone'
    source_records = json.loads(source.read_bytes())['cases']
    banks = {bank: json.loads((corpus / bank).read_bytes()) for bank in {r['bank'] for r in source_records}}
    seen = set()
    with tempfile.TemporaryDirectory(dir=work) as tmp, (work / 'programs.jsonl').open('w') as results:
        pcm = Path(tmp) / 'take.pcm'
        for number, case in enumerate(source_records):
            p = banks[case['bank']][case['preset_index']]
            address = (Path(case['bank']).stem, p['position'])
            if address in seen or p['uid'] != case['uid']:
                raise ValueError(f'factory identity mismatch: {address}')
            seen.add(address)
            subprocess.run([exe, fixture / 'public-morphs.f32', fixture / 'compact.bin',
                            address[0], str(address[1]), p['name'], pcm], check=True)
            ref = references[f'preset-{number:02}-original.f32le.pcm']
            if pcm.stat().st_size != ref['bytes'] or digest(pcm) != ref['sha256']:
                raise ValueError(f'original PCM mismatch: {address}')
            results.write(json.dumps({'bank': address[0], 'position': address[1], 'original_index': number,
                                      'pcm_sha256': ref['sha256']}, ensure_ascii=False) + '\n')
            results.flush()
    if len(seen) != 694:
        raise ValueError('incomplete bank comparison')
    summary = {'programs': len(seen), 'frames_per_program': 96000, 'channels': 2,
               'matched_stereo_frames': len(seen)*96000, 'sample_rate_hz': 48000,
               'renderer_sha256': digest(exe), 'factory_inputs_sha256': digest(base / 'FACTORY-INPUTS.json'),
               'bank_include_sha256': digest(work / 's1_factory_bank.inc'),
               'programs_sha256': digest(work / 'programs.jsonl'), 'reference_manifest_sha256': digest(manifest)}
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
