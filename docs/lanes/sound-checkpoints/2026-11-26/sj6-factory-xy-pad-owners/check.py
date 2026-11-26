from pathlib import Path
import collections,hashlib,io,json,struct,sys,tarfile,zipfile
import numpy as np
import macho
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE,UcError
from unicorn.arm64_const import *
image,bank,archive=map(Path,sys.argv[1:]);B=0x100000000
sha=lambda d:hashlib.sha256(d).hexdigest()
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
im=macho.Image(image); img=im.b
def rel(a):return a+struct.unpack_from('<i',img,a-B)[0]
def cstr(a):return img[a-B:img.index(0,a-B)].decode()
def fields(desc):
 f=rel(desc+16); n=struct.unpack_from('<I',img,f-B+12)[0]
 return [cstr(rel(f+16+12*i+8)) for i in range(n)]
assert cstr(rel(0x1006b8b60+8))=='PadType'
cases=fields(0x1006b8b60)
assert cases==['arp','delay','filter','lfo1','lfo2','lfo3','reverb','oscPitch','tremolo','unison']
base=fields(0x1006b2aa8)
assert base[203:205]==['pad1Destination','pad2Destination']
conductor=fields(0x1006bcc48)
assert conductor[320:325]==['pad1Type','pad2Type','lockPadTypes','oldPad1Type','oldPad2Type']
controller=fields(0x1006c0acc)
assert set(['touchPad1','touchPad2','pad1XInstParam','pad1YInstParam','pad2XInstParam','pad2YInstParam'])<=set(controller)
c=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
def at(a):
 i=next(c.disasm(img[a-B:a-B+4],a));return [i.mnemonic,i.op_str]
for id,reg,callback,ivar,branch,target in [(203,0x10025ee10,0x100274c6c,0xc10,0x100274cf0,0x1000f160c),(204,0x10025ee20,0x100274d20,0xc18,0x100274da4,0x1000f1684)]:
 for adr in (reg,callback):assert at(adr)==['ldr',f'x8, [x8, #{ivar:#x}]']
 assert at(branch)==['bl',f'#{target:#x}']
bind={a:im.imports[o][0] for a,o,_ in im.binds}
assert bind[0x10072d838]=='_$sSYMp' and rel(0x10062b604)==0x1006b8b60
assert rel(0x10062b620)==0x1000e861c and rel(0x10062b628)==0x10000b538
assert at(0x1000e8630)==['bl','#0x1000e86c8']
assert at(0x1000e86cc)==['cmp','x0, #0xa'] and at(0x10000b538)==['ldrb','w9, [x20]']
assert at(0x10026f618)==['ldr','x8, [x8, #0x3a0]']
assert at(0x10026f624)==['b.ne','#0x10026f798']
for a,v in ((0x10026f62c,0xc10),(0x10026f638,0x3a8),(0x10026f6e4,0xc18),(0x10026f6f0,0x3b0)):
 assert at(a)==['ldr',f'x8, [x8, #{v:#x}]']
u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
for page in [0x1000f1000,0x1000e8000,0x10000b000]:
 u.mem_map(page,4096);u.mem_write(page,img[page-B:page-B+4096])
for page in [0x10082f000,0x300000,0x400000]:u.mem_map(page,4096)
obj,stop=0x300100,0x400100
for i,a in enumerate([0x10082f390,0x10082f398]):u.mem_write(a,struct.pack('<Q',16+i))
def reset(): u.mem_write(obj+16,bytes([0xff,0xff]))
def write(owner,q):
 u.reg_write(UC_ARM64_REG_SP,0x300ff0);u.reg_write(UC_ARM64_REG_LR,stop)
 u.reg_write(UC_ARM64_REG_X20,obj);u.reg_write(UC_ARM64_REG_S0,struct.unpack('<I',struct.pack('<f',q))[0])
 u.emu_start([0x1000f160c,0x1000f1684][owner],stop,count=200)
def actual():return list(u.mem_read(obj+16,2))
arms=[]
for owner in (0,1):
 for q in [*range(10),-1,10,12,3.9]:
  reset();write(owner,q); expected=int(q) if 0<=int(q)<10 else [4,2][owner]
  assert actual()[owner]==expected and actual()[1-owner]==255
  arms.append([203+owner,q,expected])
for x,y in [(3,6),(3,2),(9,1),(0,8)]:
 reset();write(0,x);write(1,y);assert actual()==[x,y]
 assert all(0<=z<len(cases) for z in actual())
assert sha(bank.read_bytes())=='ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'
controls=['neonpad1Destination','neonpad2Destination']
records={}
with zipfile.ZipFile(bank) as z:
 for n in z.namelist():
  if n.startswith('J6presets/') and n.endswith('.superj6.bank.json'):
   for p in json.loads(z.read(n))['presets']:
    a={f['identifier']:f['value'] for f in p['parameterValues']};assert all(a[c] in range(10) for c in controls)
    records[p['uid']]=[a[c] for c in controls]
assert len(records)==640
assert sha(archive.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
signed=[]
with tarfile.open(archive) as f:
 for i in range(3):
  p=f'./program-{i}-authored/';a=json.load(f.extractfile(p+'capture.json'))
  x=np.load(io.BytesIO(f.extractfile(p+'output.npy').read()),allow_pickle=False)
  assert sha(x.tobytes())==a['pcm_sha256'] and a['active']==a['after'] and a['component']=='aumu/j6ct/AuPr'
  r={int(v['address']):(v['identifier'],v['value']) for v in a['active']}
  pair=[r[203][1],r[204][1]]
  assert [r[203][0],r[204][0]]==controls and pair==records[a['wanted']['program_uid']]
  reset();write(0,pair[0]);write(1,pair[1]);assert actual()==pair
  signed.append(dict(pair=pair,cases=[cases[j] for j in pair],pcm_sha256=a['pcm_sha256']))
print(json.dumps(dict(cases=cases,executed_controls=arms,archived_bank_total=len(records),
 both_nonzero=sum(all(q) for q in records.values()),
 distinct=sum(x!=y for x,y in records.values()),signed=signed),indent=2))
