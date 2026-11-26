import hashlib,tarfile,json,sys,io
import numpy as np
from pathlib import Path
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
image,archive=map(Path,sys.argv[1:]);B=0x100000000
img=image.read_bytes(); sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(img)=='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
assert sha(archive.read_bytes())=='c117eb6f2c51131fead72a31313f7d5d79aa2e54d05fb196b3016cba016de55e'
c=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
def at(a):
 i=next(c.disasm(img[a-B:a-B+4],a));return [i.mnemonic,i.op_str]
pairs=[
 (107,0x10025edc0,0x100273744,0xbc8,0x1002737c8,0x1000f1440),
 (108,0x10025edd0,0x1002737f8,0xbd8,0x10027387c,0x1000f1500),
 (109,0x10025ede0,0x100273898,0xbe0,None,None),
 (110,None,0x1002738ec,0xbd0,0x100273970,0x1000f1598)]
for id,reg,recv,ivar,call,dest in pairs:
 for adr in (reg,recv):
  if adr: assert at(adr)==['ldr',f'x8, [x8, #{ivar:#x}]'],(id,hex(adr),at(adr))
 if call:assert at(call)==['bl',f'#{dest:#x}']
assert at(0x100262100)==['mov','w8, #0x6e']
# Exact metadata-only branch falls into the next ivar getter, no setter call.
calls=[(hex(a),at(a)[1]) for a in range(0x100273894,0x1002738e8,4) if at(a)[0] in ('b','bl')]
assert {z for _,z in calls}=={'#0x1005fa5a8','#0x1002c27d4','#0x1005fa590','#0x1005fa344','#0x1005fe420','#0x1005fa050','#0x1005f81b4','#0x1005fa020'}
expected={107:'neoncrushEnable',108:'neoncrushAmount',109:'neoncrushBitRate'}
with tarfile.open(archive) as f:
 out={}
 for label,control in [('baseline',None),('enable',107),('mix',108),('amount',109)]:
  p='./native/live/native-jupiter-sine-gain-authored-crusher-live-'+label+'/'
  x=json.load(f.extractfile(p+'capture.json'))
  y=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
  assert sha(y.tobytes())==x['pcm_sha256']
  assert x['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
  assert x['component']=='aumu/j6ct/AuPr' and str(x['build'])=='9'
  p={int(v['address']):v['identifier'] for v in x['parameters']}
  assert {i:p.get(i) for i in expected}==expected and 110 not in p
  assert [int(y['address']) for y in x['live_steps']]==([] if control is None else [control]*2)
  out[label]=x['pcm_sha256']
 ref=json.load(f.extractfile('./live-scored.json'))
 assert ref['mix']['relative_to_own_initial'][2]['dB'] < -1.0
 assert abs(ref['amount']['relative_to_own_initial'][2]['dB'])<.02
print(json.dumps(dict(digest=sha(img),registered=expected,unregistered=[110],source_map=pairs,
    signed_native_pcm=out,metadata_only_109_calls=calls,
    signed_108_and_109_dB=[ref[k]['relative_to_own_initial'][2]['dB'] for k in ('mix','amount')]),indent=2))
