import sys,hashlib,struct,math,json,tarfile
from pathlib import Path
import macho
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
p,a,leaf=map(Path,sys.argv[1:])
def pin(path,sha):
 b=path.read_bytes(); assert hashlib.sha256(b).hexdigest()==sha; return b
b=pin(p,'6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850')
macho.EXPECTED_SHA=hashlib.sha256(b).hexdigest()
image=macho.Image(p)
assert {vm:image.imports[i][0] for vm,i,_ in image.binds if vm in (0x100755788,0x100755b80)}=={0x100755788:"_objc_msgSend",0x100755b80:"_pow"}
base=0x100000000
c=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
def code(lo,end):return {x.address:[x.mnemonic,x.op_str] for x in c.disasm(b[lo-base:end-base],lo)}
def selector(ref):
 ptr=struct.unpack_from('<Q',b,ref-base)[0]; off=ptr&0xffffff
 return b[off:b.index(0,off)].decode()
refs={hex(q):selector(q) for q in (0x10079b880,0x10079b280,0x10079bc60,0x10079b488,0x10079b500)}
assert list(refs.values())[:3]==['setMix:','setDecimation:','setRounding:']
arm=code(0x1000f1440,0x1000f160c)
mapper=code(0x1004a67d0,0x1004a6824)
amap={q:arm[q] for q in (0x1000f154c,0x1000f1540,0x1000f1554,0x1000f1564,0x1000f156c,0x1000f1594,0x1000f15e4,0x1000f15ec,0x1000f15f4)}
assert amap[0x1000f15e4]==['fmov','d2, #1.50000000']
assert [arm[q][1] for q in (0x1000f156c,0x1000f1594,0x1000f15f4)]==['#0x100603c40','#0x1006063c0','#0x100605440']
assert [mapper[q][0] for q in (0x1004a6814,0x1004a6818,0x1004a681c)]==['bl','fmul','fadd']
caller=code(0x100273750,0x100273988)
assert [caller[q][1] for q in (0x1002737c8,0x10027387c,0x100273970)]==['#0x1000f1440','#0x1000f1500','#0x1000f1598']
pin(a,'0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603')
pfields={107:'neoncrushEnable',108:'neoncrushAmount',109:'neoncrushBitRate'}
with tarfile.open(a) as f:
 d=json.load(f.extractfile('./program-1-dry/capture.json'))
 assert d['component']=='aumu/j6ct/AuPr' and d['active']==d['after']
 assert {int(q['address']):q['identifier'] for q in d['active'] if int(q['address']) in pfields}==pfields
pin(leaf,'761eaca03fe287794c03e4f00ebd99ad97f99f2bf11ba9174a3cce7b32b636ec')
with tarfile.open(leaf) as f:
 src=f.extractfile('CaptureWarm.swift').read().decode()
 assert 'parameter(withAddress:AUParameterAddress(i))' in src
 q=json.load(f.extractfile('native/capture.json'))
 assert q['au_component']=='aufx/dist/appl' and q['native_before']==q['native_after']
 assert q['native_units']['3']['name']=='Decimation' and q['native_units']['15']['name']=='Wet/Dry Mix'
 assert not q['node_bypass']
print(json.dumps({'selectors':refs,'build9_panel_ids':pfields,'callback_edges':{hex(q):caller[q] for q in (0x1002737c8,0x10027387c,0x100273970)},'mapper_edges':{hex(q):amap[q] for q in amap},'apple_native_parameters':{k:q['native_units'][k]['name'] for k in ('3','4','5','15')}},indent=2))
