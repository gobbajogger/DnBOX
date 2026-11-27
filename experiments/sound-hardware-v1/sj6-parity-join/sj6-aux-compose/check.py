"""Source-bound Sub and Noise: original receiver owns each independent stereo bus."""
from pathlib import Path
import os,sys,json,struct,hashlib,zipfile,zlib,ctypes as C,re
import numpy as np
kind=sys.argv[1]
assert kind in ('ipa','current')
if kind=='current':
 p0=Path('sj6-mix-boundary-recheck/wrapper.py')
 exec(compile(p0.read_text().split('def sampler_owner')[0],str(p0),'exec'))
 note_on=0x100389c84;note_off=0x100389c8c
else:
 import paths,macho,sampler,emu
 from unicorn.arm64_const import UC_CPU_ARM64_MAX
 Uc=emu.Uc
 def uc(*a,**k):
  u=Uc(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
 emu.Uc=uc
 BIN=Path('sj6-ipa-source-next/au.private')
 assert macho.EXPECTED_SHA=='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
 paths.BIN=str(BIN);e=emu.Emu(str(BIN))
 note_on=0x10037d148;note_off=0x10037d150
sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(BIN.read_bytes())==macho.EXPECTED_SHA
receipt=Path('recovered-sound/checkpoint/dnbox-sound-bench/sj6/brainwave-normal-sub/four-sources.json')
config=json.loads(receipt.read_bytes())
names=('oscSampler1','oscSampler2','sampler1','noiseOsc')
archive=next(Path('ab-private/superj6').glob('*.ipa'))
with zipfile.ZipFile(archive) as z:
 def resource(suffix):
  matches=[n for n in z.namelist() if n.endswith(suffix)]
  assert len(matches)==1
  return z.read(matches[0])
 sfz=resource('/Sounds/Osc/MySub.sfz');pack=resource('/J6P8R4N2S1')
assert sha(pack)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
assert pack[:8]==b'AKJ6PCM1'
p=16;records={}
for i in range(struct.unpack_from('<I',pack,12)[0]):
 n,=struct.unpack_from('<H',pack,p);p+=2
 name=pack[p:p+n].decode().split('/')[-1];p+=n
 frames,rate,off,nb,crc=struct.unpack_from('<IIQQI',pack,p);p+=28
 assert nb==frames*2 and off+nb<=len(pack) and name not in records
 records[name]=(rate,off,nb,crc)
zones=[];group={}
for line in sfz.decode().splitlines():
 row=dict(re.findall(r'(\w+)=(\S+)',line))
 if line.strip().startswith('<group>'):group=row
 if line.strip().startswith('<region>'):zones.append(group|row)
assert len(zones)==12
def load(s):
 for r in zones:
  root=int(r['pitch_keycenter']);key=r['sample'].replace(chr(92),'/').split('/')[-1]
  rate,off,nb,crc=records[key];clip=pack[off:off+nb]
  assert zlib.crc32(clip)==crc
  s.load_sample(dict(note=root,freq=440*2**((root-69)/12),lokey=int(r['lokey']),hikey=int(r['hikey']),
                loop=r['loop_mode']!='no_loop',loop_start=float(r['loop_start']),loop_end=float(r['loop_end']),
                end=float(r['end']),rate=rate),np.frombuffer(clip,'<i2').astype('f4')/32768.)
 s.build_keymap()
def owner(role,noise):
 d=sampler.SamplerDSP(e,44100)
 if role=='sampler1':load(d)
 else:d.load_placeholder()
 for address,value in config['sampler_control_writes'][role]:d.set_param(address,value,True)
 if kind=='current' and role.startswith('oscSampler'):
  e.call(0x1003898c4,d.dsp,4);e.call(0x1003898a4,d.dsp,2)
 if role=='noiseOsc' and noise is not None:
  # Direct native physical positive control, not an authored Brainwave edit.
  d.set_param(0,noise,True)
 return d
lib=C.CDLL(os.environ['ORIGINAL_SO'],mode=os.RTLD_LAZY)
F=C.c_float;P=C.POINTER(F)
class Block(C.Structure):
 _fields_=[('left',P*4),('right',P*4),('volume',F*4),('pan',F*4),('trim',F),('frames',C.c_uint)]
run=lib.sj6_probe;run.argtypes=[C.POINTER(Block),P,P,C.c_uint,F];run.restype=C.c_int
def ptr(x):return x.ctypes.data_as(P)
trim=np.float32(config['osc_section_props']['volume'])
assert trim==np.float32(.8670601844787598)
def render(noise):
 ds=[owner(n,noise) for n in names]
 for _ in range(128):
  for d in ds:d.render(128)
 # Scope a genuine accepted note/off from the held warm cue; same events all four roles.
 rows={k:[hashlib.sha256(),0.,0.,0] for k in (*names,'mix')}
 e.rand_state=0x12345678
 for stage,count in (('held',22050),('release',8192)):
  for d in ds:
   if stage=='held':e.call(note_on,d.dsp,36,86,doubles=(0.,))
   else:e.call(note_off,d.dsp,36,0)
  for start in range(0,count,128):
   n=min(count-start,128)
   v=[d.render(n) for d in ds]
   mask=[('full',v),('sub_only',[np.zeros_like(v[0]),np.zeros_like(v[1]),v[2],np.zeros_like(v[3])]),
         ('noise_only',[np.zeros_like(v[0]),np.zeros_like(v[1]),np.zeros_like(v[2]),v[3]])]
   for label,lanes in mask:
    b=Block((P*4)(*[ptr(a[0]) for a in lanes]),(P*4)(*[ptr(a[1]) for a in lanes]),(F*4)(1,1,1,1),(F*4)(),trim,n)
    out=np.zeros((2,n),dtype=np.float32)
    assert run(C.byref(b),ptr(out[0]),ptr(out[1]),n,1.)==1
    model=(((lanes[0]+lanes[1])+lanes[2])+lanes[3])*trim
    assert np.array_equal(out.view('u4'),model.view('u4')),(label,stage,start)
    if label=='full':
     for k,x in zip(rows,(*v,out)):
      z=rows[k];z[0].update(x.tobytes());z[1]+=np.sum(x.astype('f8')**2);z[2]=max(z[2],float(np.max(abs(x))));z[3]+=x.size
 return {k:dict(signed_sha=z[0].hexdigest(),rms=(z[1]/z[3])**.5,peak=z[2],words=z[3]) for k,z in rows.items()}
auth=render(None)
active=render(.25)
assert not e.unknown,e.unknown
assert auth['sampler1']['peak']>0 and auth['noiseOsc']['peak']==0 and active['noiseOsc']['peak']>0
assert auth['sampler1']['signed_sha']==active['sampler1']['signed_sha']
print(json.dumps(dict(scope='Actual independent '+kind+' ABI receivers; exact verified private Sub pack/SFZ, pinned previously reached controls projected to receivers. Authored Noise muted. Positive direct physical fader .25; emulator random shim, do not claim installed exact random PCM or host production port. External original source owners; actual guarded compact render_backend, regular dry/chorus off, trim sourced, master unity. No sample outputs saved.',original=macho.EXPECTED_SHA,controls=sha(receipt.read_bytes()),signed_checks=3*(22050+8192)*2*2,authored=auth,positive_noise=active),indent=2))
