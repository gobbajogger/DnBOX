"""Build an original Synth One factory renderer with a delay-only substitution."""
import argparse
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent


def build(repo, factory, out):
    out.mkdir(parents=True, exist_ok=True)
    text = (repo / 'tests/fixtures/synthone-factory/fullfx/source_fx.hpp').read_text()
    start = text.index('        sp_vdelay_compute(sp, delayL,')
    stop = text.index('        // DELAY MIXER', start)
    calls = text[start:stop]
    assert calls.count('sp_vdelay_compute') == 4
    text = text[:start] + '''
        Trial &trial = *active_trial;
        trial.input[0] = delayInputLowPassOutL;
        trial.input[1] = delayInputLowPassOutR;
        if (trial.mode == 0) {
''' + calls + '''
        } else if (trial.mode == 1) {
            trial.full->frame(delayInputLowPassOutL, delayInputLowPassOutR,
                              parameters[delayTime], parameters[delayFeedback], delayOutL, delayOutRR);
        } else {
            trial.half->frame(delayInputLowPassOutL, delayInputLowPassOutR,
                              parameters[delayTime], parameters[delayFeedback], delayOutL, delayOutRR);
        }
        trial.wet[0] = delayOutL;
        trial.wet[1] = delayOutRR;
''' + text[stop:]
    (out / 'capture_fx.hpp').write_text(text)
    cc = json.loads((factory / 'factory-build-commands.json').read_text())
    compile = next(row.copy() for row in cc if any(a.endswith('/preset_audio.cpp') for a in row))
    compile[compile.index('-c') + 1] = str(HERE / 'music.cpp')
    compile[compile.index('-o') + 1] = str(out / 'music.o')
    compile[compile.index('-MF') + 1] = str(out / 'music.d')
    compile.insert(1, '-I' + str(out))
    subprocess.run(compile, check=True)
    link = cc[-1].copy()
    link[link.index(str(factory / 'factory-6.o'))] = str(out / 'music.o')
    link[link.index('-o') + 1] = str(out / 'probe')
    subprocess.run(link, check=True)


if __name__ == '__main__':
    cli = argparse.ArgumentParser(description=__doc__)
    for name in ('repo', 'factory', 'out'):
        cli.add_argument(name, type=Path)
    args = cli.parse_args()
    build(args.repo.resolve(), args.factory.resolve(), args.out.resolve())
