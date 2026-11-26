#!/usr/bin/env python3
"""Whole-graph comparison: original Synth One source vs DnBOX host renderer.
No alignment, gain fitting, audio resampling, or parameter edits."""
from argparse import ArgumentParser
from concurrent.futures import ThreadPoolExecutor
from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory
from subprocess import run
import json,os,struct,sys
import numpy as np
a=ArgumentParser()
a.add_argument("reference",type=Path); a.add_argument("candidate",type=Path)
a.add_argument("waves",type=Path); a.add_argument("ref_commands",type=Path); a.add_argument("candidate_pack",type=Path)
a.add_argument("out",type=Path); a.add_argument("--ids",nargs="*",type=int)
a.add_argument("--jobs",type=int,default=4)
a.add_argument("--identity",type=Path,required=True)
a.add_argument("--same-note-tape",action="store_true")
a.add_argument("--tuning-dir",type=Path)
a.add_argument("--mode",choices=("wet","dry"),default="wet")
v=a.parse_args()
def digest(p): return sha256(p.read_bytes()).hexdigest()
tuned=json.loads((v.tuning_dir/"index.json").read_text()) if v.tuning_dir else {}
identity=json.loads(v.identity.read_text())
assert identity["exact_command_occurrences"]==identity["count"]
assert digest(v.ref_commands)==identity["aligned_sha256"],"unaligned reference commands"
assert all(x.is_file() for x in (v.reference,v.candidate,v.waves,v.ref_commands,v.candidate_pack))
def db(x,y): return 10*np.log10(np.maximum(x,1e-40)/np.maximum(y,1e-40))
def power(x): return np.mean(x*x,axis=0)
def summary(ref,trial):
    s={}
    for name,sl in [('attack',slice(0,4410)),('held',slice(0,198450)),('release',slice(198450,None)),('last_second',slice(-44100,None)),('whole',slice(None))]:
        a,b=ref[sl],trial[sl]
        s[name]={'residual_dbr':db(power(b-a),power(a)).tolist(),'level_db':db(power(b),power(a)).tolist(),
                 'reference_rms':np.sqrt(power(a)).tolist()}
    def spectrum(x):
        d=x[-44100:]*np.hanning(44100)[:,None]
        f=abs(np.fft.rfft(d,axis=0))**2
        freqs=np.fft.rfftfreq(len(d),d=1/44100)
        edges=[20,80,160,320,640,1280,2560,5120,10240,18000]
        return np.array([f[(freqs>=lo)&(freqs<hi)].sum(axis=0) for lo,hi in zip(edges,edges[1:])])
    r,c=spectrum(ref),spectrum(trial)
    active=r>r.max(axis=0)*1e-8
    d=db(c,r)
    s['tail_spectral_rms_db']=[float(np.sqrt(np.mean(d[active[:,c],c]**2))) if active[:,c].any() else 0. for c in range(2)]
    return s
def render(i):
    with TemporaryDirectory(prefix='dnbox-source-ab-') as temp:
        t=Path(temp)
        excluded={'WIDTH_TRACE','WIDEN_TRACE','ROOM_TRACE','ROOM_RETURN','ROOM_FEED','S1_AUTHORED_TUNING_F32','S1_AUTHORED_TUNING_NPO'}
        environment={k:e for k,e in os.environ.items() if k not in excluded}
        environment['S1_EXACT_MORPH_WAVES']=str(v.waves.resolve())
        table=tuned.get(str(i))
        if table:
            authored=v.tuning_dir/table['file']
            assert digest(authored)==table['sha256']
            environment.update(S1_AUTHORED_TUNING_F32=str(authored.resolve()),S1_AUTHORED_TUNING_NPO=str(table['npo']))
        aa=t/'reference.f32';bb=t/'candidate.f32'
        wanted=t/'reference.events';got=t/'candidate.events'
        rcmd=[str(v.reference),str(v.waves),str(v.ref_commands),str(i),str(aa),v.mode]
        ccmd=[str(v.candidate),str(v.candidate_pack),str(i),str(bb),v.mode]
        if v.same_note_tape:
            rcmd += [str(wanted)]
            ccmd += [str(wanted),str(got)]
        r=run(rcmd,env=environment,capture_output=True,text=True,timeout=60,check=True)
        c=run(ccmd,env=environment,capture_output=True,text=True,timeout=60,check=True)
        events=None
        if v.same_note_tape:
            assert wanted.read_bytes()==got.read_bytes(),"rendered note events differ"
            events=len(wanted.read_text().splitlines())-1
        candidate_label=c.stdout.splitlines()[0].removeprefix(f"{i}: ").rsplit(" arena ",1)[0]
        assert candidate_label==identity["rows"][i]["label"],"candidate row/name does not match decoded pack"
        pcm=lambda p: np.fromfile(p,dtype='<f4').reshape(-1,2).astype('f8')
        a,b=pcm(aa),pcm(bb)
        assert a.shape==b.shape==(264600,2) and np.isfinite(a).all() and np.isfinite(b).all()
        return {'id':i,'label':c.stdout.splitlines()[0], 'ref_sha256':digest(aa),'trial_sha256':digest(bb),'same_note_events':events,'authored_tuning':table is not None, 'metrics':summary(a,b)}
with v.ref_commands.open('rb') as f:
    magic,n=struct.unpack('<2I',f.read(8)); assert magic==0x53315043
ids=v.ids if v.ids is not None and v.ids else list(range(n))
assert ids and all(0<=i<n for i in ids) and len(set(ids))==len(ids)
with v.out.open('w') as out, ThreadPoolExecutor(max_workers=v.jobs) as ex:
    out.write(json.dumps({'scope':'Whole stereo cold-start six-note host renders, in matching command ordinal. Optional authenticated authored tuning and note tape; no project change, hardware, or fit.',
                         'mode':v.mode,'same_note_tape':v.same_note_tape,'tuning_rows':len(tuned),'count':len(ids), 'source_bank_rows':n,'identity_sha256':digest(v.identity),'inputs':{str(p):digest(p) for p in [v.reference,v.candidate,v.waves,v.ref_commands,v.candidate_pack]}})+'\n')
    for i,task in zip(ids,[ex.submit(render,i) for i in ids]):
        try: obj=task.result()
        except Exception as e: obj={'id':i,'error':str(e),'stderr':getattr(e,'stderr',None)}
        out.write(json.dumps(obj,allow_nan=False)+'\n');out.flush()
