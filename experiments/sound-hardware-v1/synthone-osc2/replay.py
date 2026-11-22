"""Compare all reachable stock panel effects on INIT with each original factory program."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FRAMES = 132300
IDS = [34, 121, 35, 36, 59, 13, 14, 15, 45, 46, 47, 116, 117, 118, 119, 32, 122, 125, 128, 131, 134]
sys.path.insert(0, str(ROOT / 'tools'))
from build import ORIGINAL_SYNTH_UNITS, ORIGINAL_SYNTH_BACKENDS


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def compile_producers(build, out, crush_lfo, voice, sub, osc2):
    commands = json.loads((build / 'factory-build-commands.json').read_text())
    cc = next(cmd.copy() for cmd in commands if any(a.endswith('/preset_audio.cpp') for a in cmd))
    cc[cc.index('-c')+1] = str(HERE.parent / 'synthone-factory-projection' / 'original.cpp')
    cc[cc.index('-o')+1] = str(out / 'original.o')
    cc[cc.index('-MF')+1] = str(out / 'original.d')
    subprocess.run(cc, check=True)
    link = commands[-1].copy()
    link[link.index(str(build / 'factory-6.o'))] = str(out / 'original.o')
    link[link.index('-o')+1] = str(out / 'original')
    subprocess.run(link, check=True)
    flags = ['-std=c11', '-O2', '-fno-fast-math', '-ffp-contract=off', '-ffunction-sections',
             '-fdata-sections', '-DX0X_HOST', f'-DX0X_SYNTHONE_PATCH={int(voice)}', '-DX0X_ORIGINAL_SYNTH=1',
             '-DX0X_WOBBLE=0', '-DX0X_DEV_WOBBLE=0', '-DORIGINAL_USE_SOFT_FMA', '-DMIAMI_FX_NO_HOST_FENV',
             '-Iexperiments/original_synth', '-Iexperiments/wobble', '-Ifirmware/src',
             '-Ifirmware/src/app', '-Ifirmware/src/dsp', '-Ibuild/gen']
    flags.append('-DPROJECT_CRUSH_LFO='+str(int(crush_lfo)))
    if osc2:
        flags.append('-DS1_PROJ_OSC2=1')
    elif sub:
        flags.append('-DS1_PROJ_SUB=1')
    subprocess.run([os.environ.get('CC','cc'), *flags, HERE/'shipping.c', *ORIGINAL_SYNTH_UNITS,
                    *ORIGINAL_SYNTH_BACKENDS, '-Wl,--gc-sections', '-lm', '-o', out/'shipping'],
                   cwd=ROOT, check=True)
    fixture = ROOT/'tests/fixtures/synthone-factory'
    subprocess.run([os.environ.get('CXX','c++'), '-O2', '-std=c++17', '-I'+str(fixture/'frontend'),
                    '-I'+str(fixture/'fullfx/compat'), HERE.parent/'synthone-autopan/factory-controls.cpp',
                    '-o', out/'controls'], check=True)


def panel_targets(build, out, cases, crush_lfo, voice_ids):
    command_file = build/'commands/preset-commands.bin'
    streams = []
    for option in ([], ['--tremolo'], ['--phaser'], ['--master']):
        lines = subprocess.check_output([out/'controls', command_file, *option], text=True).splitlines()
        assert len(lines) == len(cases) == 694
        streams.append([[float.fromhex(x) if 'p' in x else float(x) for x in row.split()] for row in lines])
    rows = []
    for i, case in enumerate(cases):
        controls = dict(case['commands'])
        rows.append([controls[34], controls[121], *streams[0][i], *streams[1][i],
                     *streams[2][i], *streams[3][i], *([controls[58]] if crush_lfo else []),
                     *(controls[parameter] for parameter in voice_ids)])
    targets = np.asarray(rows, dtype='<f4')
    assert targets.shape == (694, len(IDS)+int(crush_lfo)+len(voice_ids)) and np.isfinite(targets).all()
    targets.tofile(out/'panel-targets.f32')


def score(a, b):
    a, b = a.astype(float), b.astype(float)
    def spectrum(v):
        w = np.lib.stride_tricks.sliding_window_view(v,512)[::128]
        return np.sqrt(np.mean(np.abs(np.fft.rfft(w*np.hanning(512),axis=1))**2,axis=0))
    ref, test = spectrum(a), spectrum(b)
    floor = max(ref.max()*1e-4, 1e-12)
    keep = (ref > floor) | (test > floor)
    spectral = np.sqrt(np.mean((20*np.log10(np.maximum(test[keep],floor) /
                                           np.maximum(ref[keep],floor)))**2)) if keep.any() else 0.
    level = 10*np.log10(max(1e-20,np.mean(b*b)) / max(1e-20,np.mean(a*a)))
    active = bool(np.count_nonzero(a) and np.count_nonzero(b))
    return {'level_db':float(level), 'spectral_db':float(spectral), 'active':active,
            'pass':bool(active and abs(level)<=1. and spectral<=3.)}


def render_one(i, case, build, out, temp):
    ref, stock, knobs = (temp/n for n in ('original.f32','shipping.f32','panel.u8'))
    originals = build/'factory-inputs'
    subprocess.run([out/'original', originals/'public-morphs.f32', originals/'compact.bin',
                    build/'commands/preset-commands.bin', str(i), ref], check=True)
    subprocess.run([out/'shipping', out/'panel-targets.f32', str(i), stock, knobs], check=True)
    a, b = (np.fromfile(p,dtype='<f4').reshape(FRAMES,2) for p in (ref,stock))
    assert np.isfinite(a).all() and np.isfinite(b).all()
    sides = [score(a[:,c], b[:,c]) for c in (0,1)]
    return {'index':i, 'bank':case['bank'], 'bank_position':case['preset_index'], 'name':case['name'],
            'original_sha256':digest(a.tobytes()), 'stock_sha256':digest(b.tobytes()),
            'panel':list(knobs.read_bytes()), 'channels':sides, 'pass':all(s['pass'] for s in sides)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('factory_build', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--count', default=694, type=int)
    parser.add_argument('--crush-lfo', action='store_true', help='Include original parameter 58 on firmware with the crush LFO route')
    parser.add_argument('--voice', action='store_true', help='Also project the editable voice patch through the persisted firmware cell')
    parser.add_argument('--sub', action='store_true', help='Include sub level, octave and waveform when projecting voice edits')
    parser.add_argument('--osc2', action='store_true', help='Include raw oscillator 2 level, shape, tune and detune')
    parser.add_argument('--jobs', default=1, type=int, help='Render independent programs concurrently')
    args=parser.parse_args()
    if args.osc2 and not args.sub:
        parser.error('--osc2 requires --sub')
    if args.sub and not args.voice:
        parser.error('--sub requires --voice')
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    assert 1 <= args.count <= 694
    build, out = args.factory_build.resolve(), args.output.resolve()
    out.mkdir(parents=True,exist_ok=True)
    cases = json.loads((build/'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
    compile_producers(build, out, args.crush_lfo, args.voice, args.sub, args.osc2)
    voice_ids = []
    if args.voice:
        voice_ids = [int(id_) for id_ in subprocess.check_output(
            [out/'shipping', '--voice-ids'], text=True).splitlines()]
        if not voice_ids or len(set(voice_ids)) != len(voice_ids):
            raise ValueError('Missing or duplicate Synth One voice descriptors')
    panel_targets(build, out, cases, args.crush_lfo, voice_ids)
    def render(item):
        i, case = item
        with tempfile.TemporaryDirectory(dir=out) as temp:
            return render_one(i, case, build, out, Path(temp))

    cases_out = []
    with ThreadPoolExecutor(max_workers=args.jobs) as pool, (out/'rows.jsonl').open('w') as journal:
        for row in pool.map(render, enumerate(cases[:args.count])):
            cases_out.append(row)
            journal.write(json.dumps(row, ensure_ascii=False) + '\n')
            journal.flush()
    result = {'rate':44100,'frames_per_program':FRAMES,'programs':len(cases_out),
              'original_frontend_writes_per_program':144,'projected_original_control_ids':IDS+([58] if args.crush_lfo else [])+voice_ids,
              'projected_voice_ids':voice_ids,
              'frames_per_arm':len(cases_out)*FRAMES,
              'reference_checksums':{n:digest((build/n).read_bytes()) for n in
                  ('factory-inputs/public-morphs.f32','factory-inputs/compact.bin',
                   'TESTED-SOURCES.json','FACTORY-INPUTS.json')},
              'source':'AudioKit/AudioKitSynthOne@6466a3715c96b7ecf1dd255a218cbd571408a314',
              'comparison':('Complete original factory voice and effects versus host firmware INIT ' \
                            'with nearest saved FX settings and editable voice controls.' if args.voice else
                            'Complete original factory voice and effects versus shipping INIT with nearest saved FX panel settings, host-compiled ordinary firmware backend.'),
              'row_file_sha256':digest((out/'rows.jsonl').read_bytes()),
              'commands_sha256':digest((build/'commands/preset-commands.bin').read_bytes()),
              'stereo_pass':sum(r['pass'] for r in cases_out),
              'left_pass':sum(r['channels'][0]['pass'] for r in cases_out),
              'right_pass':sum(r['channels'][1]['pass'] for r in cases_out)}
    (out/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result),flush=True)


if __name__ == '__main__':
    main()
