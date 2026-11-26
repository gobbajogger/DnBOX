"""Rebuild the guarded projection in memory and validate the loaded synthetic envelope."""
import importlib.util,json,struct,sys,zlib
from pathlib import Path
here=Path(__file__).resolve().parent
base=here.parent
sys.path.insert(0,str(base/'restored-harness-inspect/helpers'))
import guarded_replay as g
model,elf,expected,seed=sys.argv[1:]
package=base/'out'/f'DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc'
spec=importlib.util.spec_from_file_location('seed_project_reader',base/'fw/tools/fm1_rescue.py')
loader=importlib.util.module_from_spec(spec)
spec.loader.exec_module(loader)
plain,_,_=g.project(loader,model,package,Path(elf),expected)
loaded=g.regular(Path(seed))
g.ensure(len(plain)==len(loaded)==0x100000,'destination shape')
unchanged=bytearray(loaded)
required=(0,1,2,3,5,6)
parts={}
for n in required:
    p=0x9d000+8192*n
    h=loaded[p:p+32]
    magic,obj,flags,rev,size,crc,_,_,seal=struct.unpack('<IHH6I',h)
    g.ensure((magic,obj,flags,rev)==(0x30583058,n,0,1) and 4<=size<=3840,'synthetic project envelope')
    b=loaded[p+256:p+256+size]
    g.ensure(zlib.crc32(b)==crc and zlib.crc32(h[:28])==seal,'synthetic project CRC')
    g.ensure(h[24:28]==h[20:24]==b'\xff'*4,'reserved project header')
    parts[n]=b
    unchanged[p:p+32]=plain[p:p+32]
    unchanged[p+256:p+256+size]=plain[p+256:p+256+size]
p=0xaf000
g.ensure(loaded[p:p+16]==b'SMK37DATAX0X\0\0\0\0','synthetic owner')
unchanged[p:p+16]=plain[p:p+16]
g.ensure(unchanged==plain,'seed has non-project modifications')
g.ensure(len(parts[0])==44 and len(parts[1])==2716,'settings and sound shape')
for n in (2,3,5,6):
    g.ensure(g.u32(parts[n],0)==13,'project stream revision')
sound=parts[1][4:]
g.ensure(sound[768:800].hex()=='400000000000000000000000000000000000000000000000c0004f530202af00' and sound[832:835]==b'A\x01\x09' and
    sound[960:992].hex()=='00fa000000000000000000008030000000000000000000000000000000000000' and
    sound[2688:2700].hex()=='3131534f0200030100000100','S1 selected/arp-off or wrong project tag')
print(json.dumps(dict(sealed_firmware=True,installed_project_owner='synthetic S1 A/arp 2 octaves',
                      all_other_projected_bytes_equal=True)))
