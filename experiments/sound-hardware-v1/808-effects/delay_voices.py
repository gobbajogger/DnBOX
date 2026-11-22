"""Compare the bounded shipping 808 echo to untouched 8W8 processing on every voice."""
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
    for i,unit in enumerate((cpp,dsp/'fxbus.c',dsp/'x0x_param.c',dsp/'drum909.c',dsp/'drum808.c',HERE/'delay_voices.c')):
        obj=out/f'{i}.o'
        subprocess.run(['c++' if i==0 else 'cc',*flags,'-c',unit,'-o',obj],check=True)
        objects.append(obj)
    exe=out/'compare'
    subprocess.run(['c++',*objects,'-lm','-o',exe],check=True)
    cases=[]
    default=[7,68,60,93,55,120]
    beats=[.125,1/6,.25,1/3,.375,.5,2/3,.75,1,4/3,1.5,2,3]
    cap=(19200-256)/44100
    for voice in [*range(16),-1]:
        for rv,mode,glue,fb in ((0,0,0,68),(0,0,0,127),(1,0,0,68),(1,1,108,68)):
            vals=default.copy();vals[1]=fb
            cases.append((voice,rv,mode,glue,vals,True))
    for voice in [1,14,-1]:
        for bpm in [45,60,90,120,180,240]:
            for i,beat in enumerate(beats):
                vals=default.copy();vals[0]=i;vals[5]=bpm
                cases.append((voice,0,0,0,vals,beat*60/bpm<=cap))
        for axis in range(1,5):
            for pos in [0,64,127]:
                vals=default.copy();vals[axis]=pos
                cases.append((voice,0,0,0,vals,True))
    rows=[]
    for voice,rv,mode,glue,values,within in cases:
        tmp=out/'pair.f32'
        subprocess.run([exe,*map(str,[voice,rv,mode,glue,*values]),tmp],check=True)
        pcm=np.fromfile(tmp,dtype='<f4').reshape(88200,4)
        tmp.unlink()
        if not np.isfinite(pcm).all() or not np.array_equal(pcm[:,2],pcm[:,3]): raise RuntimeError('invalid mono return')
        x,ref,port,_=pcm.astype('d').T
        level,spec=channel_score(ref,port,44100,[20,16000])
        row={'voice':voice,'division_feedback_tone_level_hpf_bpm':values,'reverb':rv,'drive_mode':mode,'glue':glue,'in_range':within,'level_db':level,'spectral_rms_db':spec,
             'pass':bool(abs(level)<=1 and spec<=3),'reference_sha256':hashlib.sha256(pcm[:,1].tobytes()).hexdigest()}
        if not(mode or glue) and np.max(np.abs(ref-x))>1e-8:
            level,spec=channel_score(ref-x,port-x,44100,[20,16000])
            row.update(return_level_db=level,return_spectral_rms_db=spec,return_pass=bool(abs(level)<=1 and spec<=3))
        elif not(mode or glue) and not np.array_equal(ref,port):
            raise RuntimeError('dry alignment failed')
        rows.append(row)
    valid=[r for r in rows if r['in_range']]
    report={'original':REF,'rate':44100,'frames_per_case':88200,'maximum_delay_seconds':cap,'cases':rows,
            'in_range_cases':len(valid),'full_pass':sum(r['pass'] for r in valid),
            'wet_only_pass':sum(r.get('return_pass',False) for r in valid),'wet_only_cases':sum('return_pass' in r for r in valid)}
    (out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    print({k:v for k,v in report.items() if k!='cases'})
    return int(report['full_pass']!=len(valid) or report['wet_only_pass']!=report['wet_only_cases'])

if __name__=='__main__':
    raise SystemExit(main())
