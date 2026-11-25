#!/usr/bin/env python3
"""Render only the selected two cases. f32le stereo, no gain/alignment or A/B scoring."""
import argparse, array, hashlib, json, math, os, subprocess, sys
from pathlib import Path

H=Path(__file__).resolve().parent
RATE=44100
FRAMES=264600
TAIL=198450
NAMES={62:("pad","Irresistible Forces Pad",21),113:("organ","Dark Theater Organ",72)}
MODES=("wet","dry","delay","reverb")
ART=("original","compact")
def sha(data):return hashlib.sha256(data).hexdigest()
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--original-build',type=Path,required=True)
p.add_argument('--original',type=Path,required=True)
p.add_argument('--compact',type=Path,required=True)
p.add_argument('--pack',type=Path,required=True)
p.add_argument('--compact-provenance',required=True)
p.add_argument('--out',type=Path,required=True)
a=p.parse_args()
assert sys.byteorder=='little'
orig=a.original_build.resolve()
metadata=json.loads((orig/'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())
for n,(_,name,index) in NAMES.items():
    c=metadata['cases'][n]
    assert (c['bank'],c['name'],c['preset_index'])==('Bonus.json',name,index)
cmd=orig/'commands/preset-commands.bin'
morph=orig/'factory-inputs/public-morphs.f32'
assert sha(morph.read_bytes())=='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'
assert sha(a.pack.read_bytes())=='867381277802511def492ff43db13cc3b59510bc32bc409a49329bda531b7b44'
a.out.mkdir(exist_ok=False)
a.out.chmod(0o700)
info={}
with (a.out/'render.log').open('w') as log:
    for id,(_,name,_) in NAMES.items():
        for side in ART:
            for mode in MODES:
                f=a.out/f"{id:03}-{side}-{mode}.f32le"
                if side=='compact':args=[a.compact.resolve(),a.pack.resolve(),str(id),f,mode]
                else:args=[a.original.resolve(),morph,cmd,str(id),f,mode]
                x=subprocess.run(args,stdout=subprocess.PIPE,text=True,check=True).stdout
                log.write(x);log.flush()
                if side=='compact':assert f"{id}: {name} arena " in x
                b=f.read_bytes()
                z=array.array('f');z.frombytes(b)
                assert len(b)==FRAMES*8 and all(math.isfinite(s) for s in z)
                assert any(s!=0 for s in z)
                info[f.name]={'sha256':sha(b),'frames':FRAMES,'channels':2,'peak':max(map(abs,z))}
cases=[]
for id,(kind,name,_) in NAMES.items():
    for mode in ('wet','delay','reverb'):
        captures={f'{side}-dry':f'{id:03}-{side}-dry.f32le' for side in ART}
        captures.update({f'{side}-wet':f'{id:03}-{side}-{mode}.f32le' for side in ART})
        cases.append({'name':f'synth-one-{kind}-{mode}','format':'f32le','rate':RATE,
                      'tail':f'{TAIL}:{FRAMES}','captures':captures,
                      'provenance':{'original':f"Source-derived upstream Synth One control frontend {metadata['source']} and checked Soundpipe fixture, not iOS playback",
                                    'compact':a.compact_provenance,
                                    'stimulus':f"Bonus program {id} {name}; 128-frame preroll discarded; notes 48/55/60/64/67/72, velocities 64/75/86/97/108/119, 33075 frames held each; final release 66150 frames",
                                    'effect':{'wet':'Authored wet graph vs effects-neutralized dry (includes any authored non-delay/reverb FX)',
                                              'delay':'Authored delay enable/mix only; other controllable effects neutralized; may be authored off',
                                              'reverb':'Authored reverb enable/mix only; other controllable effects neutralized; may be authored off'}[mode]}})
(a.out/'matrix.json').write_text(json.dumps({'version':1,'cases':cases},indent=2)+'\n')
(a.out/'capture-receipt.json').write_text(json.dumps({
    'scope':'Host provenance and transport only; no sign-off or gain fitting; fixture captures private',
    'commands_sha256':sha(cmd.read_bytes()),
    'pack_sha256':sha(a.pack.read_bytes()),
    'original_executable_sha256':sha(a.original.read_bytes()),
    'compact_executable_sha256':sha(a.compact.read_bytes()),
    'captures':info},indent=2)+'\n')
print(f"{len(info)} finite stereo raw renders, {FRAMES} frames each; {len(cases)} scorer cases")
