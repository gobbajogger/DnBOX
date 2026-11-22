"""Replay the original 8W8 wet snare and the shipping DnBOX stereo bus."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
REF = '94aa2716d4c2e5851fc7c5dbb4c17adfd133c997'
sys.path.insert(0, str(HERE.parent / 'sj6-factory'))
from compare import channel_score


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('upstream', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--kit-master',action='store_true',help='use the dedicated 808 kit shaper')
    parser.add_argument('--kit-glue',action='store_true',help='use the dedicated 808 glue detector')
    parser.add_argument('--kit-reverb',action='store_true',help='use the dedicated mono 808 reverb')
    parser.add_argument('--kit-delay',action='store_true',help='use the bounded private 808 delay')
    args = parser.parse_args()
    up = args.upstream.resolve()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    dsp = ROOT / 'firmware/src/dsp'
    # Materialize every original DSP file from the immutable Git tree.
    source = out / 'original'
    source.mkdir()
    for name in subprocess.check_output(['git','-C',up,'ls-tree','--name-only',REF+':src/dsp'],text=True).splitlines():
        (source / name).write_bytes(subprocess.check_output(['git','-C',up,'show',REF+':src/dsp/'+name]))
    gen = out / 'generated'
    gen.mkdir()
    subprocess.run([sys.executable, ROOT/'tools/gen_drum_samples.py', gen/'x0x_drum_samples.h'],check=True,cwd=ROOT)
    flags = ['-O2','-ffp-contract=off','-fno-fast-math','-I'+str(source),'-I'+str(dsp),'-I'+str(gen)]
    objects = []
    for i, unit in enumerate((source/'sc808_engine.cpp', dsp/'fxbus.c', dsp/'x0x_param.c', dsp/'drum909.c', dsp/'drum808.c', HERE/'wet.c')):
        obj = out/f'{i}.o'
        tool = 'c++' if i == 0 else 'cc'
        subprocess.run([tool,*flags,'-c',unit,'-o',obj],check=True)
        objects.append(obj)
    test = out/'compare'
    subprocess.run(['c++',*objects,'-lm','-o',test],check=True)
    if args.kit_master or args.kit_glue:
        sweep = out/'kit-controls'
        subprocess.run(['c++',*flags,HERE/'kit-controls.cpp',*objects[1:-1],'-lm','-o',sweep],check=True)
        (out/'kit-controls.json').write_bytes(subprocess.check_output([sweep]))
    cases = [('dry',0,0,0,0),('reverb',1,0,0,0),('delay',0,1,0,0),('glue',0,0,0,108)]
    for mode in range(1,8):
        cases.append((f'distortion-{mode}',0,0,mode,0))
    cases.append(('combined',1,1,1,108))
    rows = []
    dry_digest = None
    for name,rv,dl,dist,glue in cases:
        data = out/(name+'.f32')
        subprocess.run([test, str(rv), str(dl), str(dist), str(glue), data, str(int(args.kit_master)), str(int(args.kit_glue)), str(int(args.kit_reverb)), str(int(args.kit_delay))],check=True)
        with data.open('rb') as file:
            x, ref, left, right = np.fromfile(file,dtype='<f4').reshape(88200,4).T
        digest = hashlib.sha256(x.tobytes()).hexdigest()
        if dry_digest is None: dry_digest=digest
        if dry_digest!=digest or not np.any(x): raise ValueError('dry source changed')
        if name=='dry' and not (np.array_equal(x,ref) and np.array_equal(ref,left) and np.array_equal(ref,right)):
            raise ValueError('dry alignment failed')
        if name!='dry' and np.array_equal(ref,x): raise ValueError(f'inactive upstream effect: {name}')
        row={'case':name,'reference_sha256':hashlib.sha256(ref.tobytes()).hexdigest()}
        for channel, signal in [('left',left),('right',right)]:
            level, spectrum=channel_score(ref.astype('d'),signal.astype('d'),44100,[20,16000])
            row[channel]={'level_db':level,'spectral_rms_db':spectrum,
                          'pass':bool(abs(level)<=1 and spectrum<=3),
                          'sha256':hashlib.sha256(signal.tobytes()).hexdigest()}
        rows.append(row)
    print(json.dumps(rows,indent=2))
    (out/'results.json').write_text(json.dumps({'original':REF,'master':'808 kit' if args.kit_master else 'shared bus','reverb':'808 kit' if args.kit_reverb else 'shared bus','delay':'808 kit, maximum 429.569 ms' if args.kit_delay else 'shared bus','glue':'808 kit' if args.kit_glue else 'shared bus','frames_per_case':88200,'sample_rate':44100,
       'dry_source_sha256':dry_digest,'limits':{'absolute_level_db':1,'spectral_rms_db':3},'cases':rows},indent=2)+'\n')
    return int(not all(r['left']['pass'] and r['right']['pass'] for r in rows))


if __name__ == '__main__':
    raise SystemExit(main())
