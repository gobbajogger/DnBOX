"""Compare the shipping private 808 reverb to untouched 8W8 processing on every voice."""
import argparse, hashlib, json, subprocess, sys
from pathlib import Path
import numpy as np
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
REF='94aa2716d4c2e5851fc7c5dbb4c17adfd133c997'
sys.path.insert(0,str(HERE.parent/'sj6-factory'))
from compare import channel_score


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('upstream',type=Path)
    parser.add_argument('output',type=Path)
    args=parser.parse_args()
    out=args.output.resolve()
    out.mkdir(parents=True,exist_ok=False)
    source=out/'original'
    source.mkdir()
    for name in subprocess.check_output(['git','-C',args.upstream,'ls-tree','--name-only',REF+':src/dsp'],text=True).splitlines():
        (source/name).write_bytes(subprocess.check_output(['git','-C',args.upstream,'show',REF+':src/dsp/'+name]))
    cpp=source/'sc808_engine.cpp'
    text=cpp.read_text()
    marker='void sc808_render(sc808_engine_t *e, float *out, int frames)'
    assert text.count(marker)==1
    text=text.replace(marker,'static float *cap_x,*cap_r,*cap_d;\nextern "C" void sc808_capture(float *x,float *r,float *d) {cap_x=x;cap_r=r;cap_d=d;}\n'+marker)
    marker='        mix += sc808_verb_tick(&e->verb, send_r);'
    assert text.count(marker)==1
    cpp.write_text(text.replace(marker,'        if(cap_x){cap_x[i]=mix;cap_r[i]=send_r;cap_d[i]=send_d;}\n'+marker))
    dsp=ROOT/'firmware/src/dsp'
    gen=out/'generated'
    gen.mkdir()
    subprocess.run([sys.executable,ROOT/'tools/gen_drum_samples.py',gen/'x0x_drum_samples.h'],cwd=ROOT,check=True)
    flags=['-O2','-ffp-contract=off','-fno-fast-math','-I'+str(source),'-I'+str(dsp),'-I'+str(gen)]
    objects=[]
    for i,unit in enumerate((cpp,dsp/'fxbus.c',dsp/'x0x_param.c',dsp/'drum909.c',dsp/'drum808.c',HERE/'reverb_voices.c')):
        obj=out/f'{i}.o'
        subprocess.run(['c++' if i==0 else 'cc',*flags,'-c',unit,'-o',obj],check=True)
        objects.append(obj)
    exe=out/'compare'
    subprocess.run(['c++',*objects,'-lm','-o',exe],check=True)
    cases=[]
    base=[70,60,55,93]
    for voice in [*range(16),-1]:
        cases.append((voice,base))
    for voice in [1,-1]:
        for axis in range(4):
            for pot in [0,64,127]:
                values=base.copy();values[axis]=pot
                cases.append((voice,values))
    rows=[]
    for voice,values in cases:
        tmp=out/'pair.f32'
        subprocess.run([exe,str(voice),*map(str,values),tmp],check=True)
        pcm=np.fromfile(tmp,dtype='<f4').reshape(88200,4)
        tmp.unlink()
        if not np.isfinite(pcm).all() or not np.array_equal(pcm[:,2],pcm[:,3]): raise RuntimeError('invalid mono return')
        x,ref,port,_=pcm.astype('d').T
        level,spec=channel_score(ref,port,44100,[20,16000])
        row={'voice':voice,'decay_tone_hpf_level':values,'level_db':level,'spectral_rms_db':spec,
             'pass':bool(abs(level)<=1 and spec<=3),'reference_sha256':hashlib.sha256(pcm[:,1].tobytes()).hexdigest()}
        if np.max(np.abs(ref-x))>1e-8:
            level,spec=channel_score(ref-x,port-x,44100,[20,16000])
            row.update(return_level_db=level,return_spectral_rms_db=spec,return_pass=bool(abs(level)<=1 and spec<=3))
        elif not np.array_equal(ref,port):
            raise RuntimeError('dry alignment failed')
        rows.append(row)
    report={'original':REF,'rate':44100,'frames_per_case':88200,'cases':rows,'full_pass':sum(r['pass'] for r in rows),
            'wet_only_pass':sum(r.get('return_pass',False) for r in rows),'wet_only_cases':sum('return_pass' in r for r in rows)}
    (out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    print({k:v for k,v in report.items() if k!='cases'})
    return int(report['full_pass']!=len(rows) or report['wet_only_pass']!=report['wet_only_cases'])


if __name__=='__main__':
    raise SystemExit(main())
