"""Replay one measured Elite +1 ms image with write observation and check observer parity."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tarfile

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()

def symbols(elf):
    result = {}
    for row in subprocess.check_output(['nm','-S',str(elf)],text=True,stderr=subprocess.DEVNULL).splitlines():
        words=row.split()
        if len(words)==3:
            result[words[2]] = (int(words[0],16),0)
        if len(words)==4:
            result[words[3]] = (int(words[0],16),int(words[1],16))
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for field in ('emulator','elf','fwsc','primary','seed','causality_archive','output'):
        parser.add_argument('--'+field.replace('_','-'),required=True,type=lambda v: Path(v).resolve())
    parser.add_argument('--faders', action='store_true', help='trace all four faders and their project destinations')
    args=parser.parse_args()
    args.output.mkdir();(args.output/'work').mkdir()
    syms=symbols(args.elf)
    with tarfile.open(args.causality_archive) as arc:
        layout=json.load(arc.extractfile('layout/layout.json'))
        template=arc.extractfile('stock/observer.map').read().decode().splitlines()
    assert syms['fm1_in'][1]==layout['hal_size'] and syms['proj'][1]==layout['proj_size']
    fields=[(n,n,0,int(s)) for n,a,s in map(str.split,template) if not n.startswith('scene_')]
    fields += [('scene_enc','fm1_in',layout['enc_steps'],16),('scene_ui_time','ui',layout['ui_enc_t'],28),('scene_ui_run','ui',layout['ui_enc_run'],7),('scene_context','ui',0,2),('scene_cutoff','proj',840,1)]
    if args.faders:
        fields += [('scene_faders','fm1_in',584,4), ('scene_last','smk_faders.last',0,4),
                   ('scene_bank','smk_fbank',0,1), ('scene_target','smk_mute_intended',0,4)]
        for offset in (1583,1587,1960,1992,2024,2056,2088,2352,2353,2357):
            fields.append((f'scene_p{offset}','proj',offset,1))
    rows=[]
    for label,symbol,offset,size in fields:
        address,limit=syms[symbol]
        assert offset+size<=limit
        rows.append(f'{label} {address+offset:x} {size}')
    out=args.output;obs=out/'observer.map';obs.write_text('\n'.join(rows)+'\n')
    shutil.copyfile(args.seed,out/'flash.bin')
    primary=json.loads((args.primary/'result.json').read_text())
    scene=args.primary/'scene.keys'
    env={k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
    env.update(FM1_STRICT='1',FM1_BENCH_KEYS='@'+str(scene),FM1_CORE_LOAD_CSV=str(out/'core.csv'),FM1_CORE_LOAD=f"{syms['_c1_start'][0]:x}-{syms['_c1_end'][0]:x}",FM1_DUMPRAM=str(out/'ram.bin'),FM1_DNBOX_PCM=str(out/'stereo.s16le'),FM1_DNBOX_MAP=str(obs),FM1_DNBOX_TRACE=str(out/'trace.jsonl'))
    with (out/'run.log').open('w') as log:
        subprocess.run([args.emulator,'--fwsc',args.fwsc,'--mhz','360','--shot',str(primary['duration_ms']),'--flash',out/'flash.bin'],cwd=out,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
    files={f: [digest(p/f) for p in [args.primary,out]] for f in ['core.csv','ram.bin','flash.bin','stereo.s16le','run.log']}
    assert all(a==b for a,b in files.values()),files
    checks={'emulator':digest(args.emulator),'elf':digest(args.elf),'fwsc':digest(args.fwsc),'seed':digest(args.seed),'scene':digest(scene),'map':digest(obs),'trace':digest(out/'trace.jsonl'),'file_hashes_primary_observed':files}
    (out/'observer-check.json').write_text(json.dumps(checks,indent=2)+'\n')
    write_snapshots(out)
    if args.faders:
        write_faders(out)


def write_snapshots(out):
    with (out/'irq-snapshots.csv').open('w') as stream:
        dest=csv.writer(stream)
        dest.writerow(['dma_release','guest_ms','view','part','k1_last_ms','k1_run','k1_pending','cutoff'])
        for line in (out/'trace.jsonl').open('rb'):
            row=json.loads(line)
            if row['event']!='irq_enter':
                continue
            ms=row['cycle']/5760000.
            if ms < 4500:
                continue
            snap=row['snapshot']
            time=struct.unpack('<7I',bytes(snap['scene_ui_time']))[3]
            pending=struct.unpack('<8h',bytes(snap['scene_enc']))[4]
            view,part=snap['scene_context']
            dest.writerow([row['seq'],f'{ms:.6f}',view,part,time,snap['scene_ui_run'][3],pending,snap['scene_cutoff'][0]])


def write_faders(out):
    offsets=(1583,1587,1960,1992,2024,2056,2088,2352,2353,2357)
    with (out/'fader-snapshots.csv').open('w') as stream:
        dest=csv.writer(stream)
        dest.writerow(['dma_release','phase','guest_ms','bank','scanner','last','intended']+list(offsets))
        for line in (out/'trace.jsonl').open('rb'):
            row=json.loads(line)
            phase=row['event']
            if phase not in ('irq_enter','half'):
                continue
            ms=row.get('cycle',row.get('completion_cycle'))/5760000.
            if ms<6760 or ms>7475:
                continue
            v=row['snapshot']
            dest.writerow([row['seq'],phase,f'{ms:.6f}',v['scene_bank'][0],
                           *(' '.join(map(str,v[key])) for key in ['scene_faders','scene_last','scene_target']),
                           *(v[f'scene_p{offset}'][0] for offset in offsets)])


if __name__ == '__main__':
    main()
