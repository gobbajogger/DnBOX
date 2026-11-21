"""Check a diagnostic replay against all artifacts from the corresponding 8-second primary."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for field in ('emulator','elf','package','seed','primary','map','output'):
        parser.add_argument('--'+field,type=lambda p:Path(p).resolve(),required=True)
    a=parser.parse_args()
    syms={}
    for line in subprocess.check_output(['nm','-S',a.elf],stderr=subprocess.DEVNULL,text=True).splitlines():
        f=line.split()
        if len(f)==4:syms[f[3]]=(int(f[0],16),int(f[1],16))
        if len(f)==3:syms[f[2]]=(int(f[0],16),0)
    assert syms['fm1_in'][1]==636 and syms['fm1_ms'][1]==4 and syms['proj'][1]==36420
    primary=json.loads((a.primary/'result.json').read_text())
    assert (primary['duration_ms'],primary['events'],primary['shift_ms'],primary['strict'],primary['mhz'])==(8000,922,1,1,360)
    assert digest(a.seed)==primary['initial_flash_sha256']
    assert digest(a.primary/'scene.keys')==primary['scene_sha256']
    out=a.output;out.mkdir();(out/'work').mkdir()
    watch=[]
    for line in a.map.read_text().splitlines():
        name,_,size=line.split()
        if not name.startswith('scene_'):
            assert syms[name][1]==int(size)
            watch.append(f'{name} {syms[name][0]:x} {size}')
    for name,symbol,offset,size in [('scene_read_input','fm1_in',0,416),('scene_read_analog','fm1_in',416,220),
                                   ('scene_read_clock','fm1_ms',0,4),('scene_bank','smk_fbank',0,1),
                                   ('scene_909level','proj',1960,1),('scene_bassblevel','proj',2056,1)]:
        watch.append(f'{name} {syms[symbol][0]+offset:x} {size}')
    (out/'observer.map').write_text('\n'.join(watch)+'\n')
    shutil.copyfile(a.seed,out/'flash.bin')
    env={k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
    env.update(FM1_STRICT='1',FM1_BENCH_KEYS='@'+str(a.primary/'scene.keys'),FM1_DUMPRAM=str(out/'ram.bin'),
               FM1_DNBOX_PCM=str(out/'stereo.s16le'),FM1_CORE_LOAD_CSV=str(out/'core.csv'),
               FM1_CORE_LOAD=f"{syms['_c1_start'][0]:x}-{syms['_c1_end'][0]:x}",FM1_DNBOX_TRACE=str(out/'trace.jsonl'),
               FM1_DNBOX_MAP=str(out/'observer.map'))
    with (out/'run.log').open('w') as log:
        subprocess.run([a.emulator,'--fwsc',a.package,'--mhz','360','--shot','8000','--flash',out/'flash.bin'],
                       env=env,cwd=out,stdout=log,stderr=subprocess.STDOUT,check=True)
    files={f:{'primary':digest(a.primary/f),'diagnostic':digest(out/f)}
           for f in ('ram.bin','flash.bin','stereo.s16le','core.csv','run.log')}
    assert all(v['primary']==v['diagnostic'] for v in files.values()),files
    record=dict(emulator_sha256=digest(a.emulator),elf_sha256=digest(a.elf),package_sha256=digest(a.package),
                seed_sha256=digest(a.seed),scene_sha256=primary['scene_sha256'],trace_sha256=digest(out/'trace.jsonl'),
                map_sha256=digest(out/'observer.map'),files=files)
    (out/'parity.json').write_text(json.dumps(record,indent=2)+'\n')


if __name__=='__main__':
    main()
