"""Measure a +1 ms Elite panel capture against a corresponding stock recording."""
import argparse
import importlib.util
import json
from pathlib import Path
import struct
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('packet', 'emulator', 'elf', 'fwsc', 'output'):
        parser.add_argument('--' + name.replace('_', '-'), required=True, type=lambda p: Path(p).resolve())
    for name in ('stock-elf', 'stock-run'):
        parser.add_argument('--' + name, type=lambda p: Path(p).resolve())
    parser.add_argument('--ms', type=int, required=True)
    args = parser.parse_args()
    if bool(args.stock_run) != bool(args.stock_elf):
        parser.error('--stock-run and --stock-elf must be provided together')
    # The original packet also has a file named replay.py.
    sys.path.insert(0, str(args.packet))
    import qualify
    spec = importlib.util.spec_from_file_location('startup_replay', args.packet / 'replay.py')
    runner = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(runner)
    source = args.packet / 'heavy.keys'
    if runner.digest(source) != 'd08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271':
        raise ValueError('unexpected physical scene')
    events = [(int(t), v) for t, v in (s.split(':', 1) for s in source.read_text().strip().split(','))]
    symbols = runner.read_symbols(args.elf)
    args.output.mkdir(parents=True)
    runner.run(args, symbols, events, 'seed', 2000, 0)
    actual = runner.run(args, symbols, events, 'shot', args.ms, 1, args.output / 'seed/flash.bin')
    shot = args.output / 'shot'
    if args.stock_run is None:
        return
    stock = json.loads((args.stock_run / 'result.json').read_text())
    for key in ('duration_ms', 'shift_ms', 'scene_sha256', 'events', 'strict', 'mhz'):
        if stock[key] != actual[key]:
            raise ValueError(f'stock and candidate disagree on {key}')
    def project(directory, elf_symbols):
        return qualify.read_project((directory / 'ram.bin').read_bytes(), elf_symbols)
    stock_project = project(args.stock_run, runner.read_symbols(args.stock_elf))
    candidate_project = project(shot, symbols)
    ref = (args.stock_run / 'stereo.s16le').read_bytes()
    pcm = (shot / 'stereo.s16le').read_bytes()
    result = dict(shot=actual, emulator_sha256=runner.digest(args.emulator),
                  elf_sha256=runner.digest(args.elf), fwsc_sha256=runner.digest(args.fwsc),
                  pcm=qualify.data_check(ref, pcm, 4),
                  project=qualify.data_check(stock_project, candidate_project, 1),
                  cutoff=dict(stock=stock_project[840], candidate=candidate_project[840]),
                  different_project_bytes=sum(a != b for a, b in zip(stock_project, candidate_project)))
    frame = result['pcm']['first_different_unit']
    if frame is not None and (frame + 1) * 4 <= min(len(ref), len(pcm)):
        result['first_different_frame'] = dict(stock=struct.unpack_from('<hh', ref, frame*4),
                                               candidate=struct.unpack_from('<hh', pcm, frame*4))
    (shot / 'comparison.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
