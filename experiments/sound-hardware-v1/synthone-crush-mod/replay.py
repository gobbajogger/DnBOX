"""Score the stock mono effects against the pinned source at common saved controls."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import argparse
import hashlib
import gzip
import json
import os
import subprocess
import sys
import tempfile
import numpy as np

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
sys.path.insert(0,str(ROOT/'tools'))
from build import ORIGINAL_SYNTH_UNITS, ORIGINAL_SYNTH_BACKENDS

def prepare(build,out,projection,optional):
    source=build/'closure/master/AudioKitSynthOne/DSP/Kernel/S1DSPKernel+process.mm'
    raw=source.read_bytes()
    assert hashlib.sha256(raw).hexdigest()=='3fa142e20a4e51869b796743a61e58e04c34e73f81ca6736f5c456a81cadce53'
    s=raw.decode()
    lfo=s[s.index('        float lfo1;'):s.index('        /// MARK: ARPEGGIATOR')].strip()
    hold=s[s.index('        // BITCRUSH LFO'):s.index('        ///MARK: STEREO CHAIN')].strip()
    (out/'original-stage.inc').write_text(lfo+'\n'+hold+'\n')
    sp=ROOT/'firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe'
    subprocess.run([os.environ.get('CC','cc'),'-std=c11','-O2','-ffp-contract=off','-fno-fast-math',
        '-ffunction-sections','-fdata-sections','-DX0X_HOST','-DX0X_SYNTHONE_PATCH='+str(optional),
        '-DX0X_ORIGINAL_SYNTH=1','-DX0X_DEV_WOBBLE=0','-DX0X_WOBBLE=0','-DORIGINAL_USE_SOFT_FMA',
        '-DMIAMI_FX_NO_HOST_FENV','-DNO_LIBSNDFILE','-D_DEFAULT_SOURCE','-I'+str(out),'-I'+str(sp),
        '-I'+str(sp/'lib/dr_wav'),'-Iexperiments/original_synth','-Iexperiments/wobble',
        '-Ifirmware/src','-Ifirmware/src/app','-Ifirmware/src/dsp','-Ibuild/gen',HERE/'compare.c',
        sp/'modules/phasor.c',sp/'modules/port.c',*ORIGINAL_SYNTH_UNITS,*ORIGINAL_SYNTH_BACKENDS,
        '-Wl,--gc-sections','-lm','-o',out/'compare'],cwd=ROOT,check=True)
    cases=json.loads((build/'commands/ORIGINAL-PRESET-COMMANDS.json').read_bytes())['cases']
    scored=[json.loads(line) for line in gzip.decompress((projection/'rows.jsonl.gz').read_bytes()).splitlines()]
    assert len(cases)==len(scored)==694
    frontend=build/'factory-inputs/tests/fixtures/synthone-factory'
    controls=projection.parent/'synthone-autopan/factory-controls.cpp'
    subprocess.run([os.environ.get('CXX','c++'),'-std=c++17','-O2','-I'+str(frontend/'frontend'),
                    '-I'+str(frontend/'fullfx/compat'),controls,'-o',out/'controls'],check=True)
    lines=subprocess.check_output([out/'controls',build/'commands/preset-commands.bin','--tremolo'],text=True).splitlines()
    assert len(lines)==694
    original=[]
    rows=bytearray()
    for i,(case,previous) in enumerate(zip(cases,scored)):
        assert case['name']==previous['name'] and i==previous['index']
        p=previous['panel']
        route=int(dict(case['commands'])[58])
        rows.extend([*p[5:11],p[0],(route*127+1)//3,p[4]])
        decoded=[float.fromhex(v) if 'p' in v else float(v) for v in lines[i].split()]
        original.append([*decoded[1:7],dict(case['commands'])[34],route,decoded[0]])
    np.asarray(original,dtype='<f4').tofile(out/'authored.f32')
    (out/'panels').write_bytes(rows)
    return cases

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('factory_build',type=Path)
    parser.add_argument('output',type=Path)
    parser.add_argument('--projection',type=Path,default=HERE.parent/'synthone-factory-projection')
    parser.add_argument('--optional',choices=[0,1],type=int,default=0)
    parser.add_argument('--authored',action='store_true',help='Use unquantized original factory settings in the source arm')
    args=parser.parse_args()
    out=args.output.resolve()
    out.mkdir(parents=True,exist_ok=True)
    cases=prepare(args.factory_build.resolve(),out,args.projection.resolve(),args.optional)
    # The same per-channel musical gate as the full factory comparison.
    import importlib.util
    spec=importlib.util.spec_from_file_location('factory_projection',args.projection/'replay.py')
    projection=importlib.util.module_from_spec(spec);spec.loader.exec_module(projection)
    def run(i):
        with tempfile.TemporaryDirectory(dir=out) as tmp:
            path=Path(tmp)/'audio'
            subprocess.run([out/'compare',out/'panels',str(i),path,*([out/'authored.f32'] if args.authored else [])],check=True)
            data=np.fromfile(path,'<f4').reshape(44100,2)
            assert np.isfinite(data).all()
        error=projection.score(data[:,0],data[:,1])
        error.update(index=i,name=cases[i]['name'],route=int(dict(cases[i]['commands'])[58]),
                     different_samples=int(np.count_nonzero(data[:,0]!=data[:,1])))
        return error
    with ThreadPoolExecutor(4) as pool: result=list(pool.map(run,range(len(cases))))
    (out/'rows.json').write_text(json.dumps(result,indent=1)+'\n')
    print(json.dumps({'passed':sum(r['pass'] for r in result),'routed':sum(r['route']>0 for r in result),
                      'worst_spectral':max(r['spectral_db'] for r in result),
                      'worst_level':max(abs(r['level_db']) for r in result),
                      'failures':[r for r in result if not r['pass']]}),flush=True)

if __name__=='__main__':main()
