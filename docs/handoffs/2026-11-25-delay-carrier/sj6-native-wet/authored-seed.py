"""Create a synthetic typed Init project in a disposable emulator NOR only."""
import sys,subprocess,struct,zlib
from pathlib import Path
from seed import native_project
untouched,output,source_elf,source_ram,encoder=map(Path,sys.argv[1:6])
for p in (untouched,output): assert p.resolve().is_relative_to('/tmp')
p=bytearray(native_project(source_elf,source_ram,36420))
assert p[:8]==b'0X0P\x01\x00\x00\x00'
assert p[19]==13 and p[40+800+29:40+800+31]==b'\x03\x00'
assert p[40+2700:40+2704]==b'1UJO'
p[19]=0x70
for slot,index in enumerate((1024,1088)):
    i=40+index
    assert not any(p[i:i+32])
    p[i:i+2]=bytes((137,slot))
# Start with an actual wet profile, change feedback from 63 to 64 over USB.
i=40+1088+16;p[i:i+4]=bytes((3,45,63,127))
settings=bytearray(p[:40]);settings[27]&=~1
h=14695981039346656037
for x in bytes((16,0))+settings+p[40:]:
    h=((h^x)*1099511628211)&((1<<64)-1)
raw=bytes(p[2752:]); packed=subprocess.check_output([str(encoder),'encode'],input=raw)
assert len(packed)<4*3836-12
items=[(0,struct.pack('<I',1)+p[:40]),(1,struct.pack('<I',0x584a3656)+p[40:2752]),
       (2,struct.pack('<IQI',16,h,len(packed))+packed[:3824])]
packed=packed[3824:]
for k in (3,5,6):
    if packed: items.append((k,struct.pack('<I',16)+packed[:3836]));packed=packed[3836:]
assert not packed
nor=bytearray(untouched.read_bytes());assert len(nor)==1048576
for obj,data in items:
    off=0x9d000+obj*8192
    assert nor[off:off+4096]==b'\xff'*4096 and len(data)<=3840
    head=struct.pack('<IHH5I',0x30583058,obj,0,1,len(data),zlib.crc32(data),0xffffffff,0xffffffff)
    nor[off:off+32]=head+struct.pack('<I',zlib.crc32(head))
    nor[off+256:off+256+len(data)]=data
at=0xaf000
assert nor[at:at+16]==b'\xff'*16
nor[at:at+16]=b'SMK37DATAX0X\0\0\0\0'
with output.open('xb') as f:f.write(nor)
