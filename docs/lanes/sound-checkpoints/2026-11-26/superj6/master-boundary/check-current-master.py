#!/usr/bin/env python3
"""Verify the current Mac SuperJ6 master handoff from the installed ARM64 slice."""
import hashlib, json, mmap, re, struct, subprocess, sys
path=sys.argv[1]
def otool(*args):
    return subprocess.check_output(["otool","-arch","arm64",*args,path],text=True)
sha=hashlib.sha256(open(path,"rb").read()).hexdigest()
assert sha=="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
data=otool("-ov")
assert re.search(r"0x100629b70 200\s+name\s+0x100684c80 masterAttenuator",data)
m=data.index("_TtC8AudioKit7AKMixer\n")
assert re.search(r"\(0x10079c040\)[\s\S]{0,100}\(0x10047b638\)",data[m:m+4000])
asm=otool("-tV")
def span(begin,end,expected):
    a=asm.index(f"{begin:016x}\t")
    b=asm.index(f"{end:016x}\t",a)
    block=asm[a:b]
    for instruction in expected:
        assert instruction in block,instruction
span(0x1000ee4d4,0x1000ee510,[
    "ldr\tx0, [x8, #0xc8]", "fcvt\td8, s0","mov\tx8, #0x6042",
    "movk\tx8, #0xd0e5, lsl #16", "movk\tx8, #0xdb22, lsl #32",
    "movk\tx8, #0x3fd9, lsl #48","fmov\td1, x8","movi.2d\tv0, #0000000000000000",
    "fmov\td2, #1.00000000","mov.16b\tv3, v8","bl\t0x1004a67d0","bl\t0x100607340"])
span(0x1004a67e0,0x1004a6824,["fsub\td8, d1, d0","symbol stub for: _pow","fmul\td1, d8, d0","fadd\td0, d9, d1"])
span(0x10047b670,0x10047b694,["fcmp\td8, #0.0","fcsel\td0, d0, d8, ls","fcvt\ts0, d0","bl\t0x100605960"])
def read_sections():
    sections={}
    for section in otool("-l").split("\nSection\n")[1:]:
        x={kv[0]:kv[1] for row in section.splitlines() if len(kv:=row.split())==2}
        sections[x["sectname"]]=(int(x["addr"],16),int(x["size"],16),int(x["offset"]))
    return sections
sec=read_sections()
with open(path,"rb") as f:
    image=mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ)
    assert image[:4]==b"\xca\xfe\xba\xbe"
    arch=next(off for i in range(struct.unpack_from(">I",image,4)[0])
              for cpu,_,off,*_ in [struct.unpack_from(">IIIII",image,8+i*20)] if cpu==0x0100000c)
    def section_off(name,addr):
        base,size,off=sec[name]
        assert 0<=addr-base<size
        return arch+off+addr-base
    def selector(pc):
        w,n=struct.unpack_from("<II",image,section_off("__objc_stubs",pc))
        assert w&0x9f00001f==0x90000001 and n&0xffc003ff==0xf9400021
        delta=((w>>5)&0x7ffff)<<2 | ((w>>29)&3)
        if delta&0x100000:delta-=0x200000
        ptr=(pc&~4095)+(delta<<12)+(((n>>10)&0xfff)<<3)
        raw=struct.unpack_from("<Q",image,section_off("__objc_selrefs",ptr))[0]
        off=section_off("__objc_methname",0x100000000+(raw&0xfffffffff))
        return image[off:image.find(b"\0",off)].decode()
    names=[selector(x) for x in (0x100607340,0x100605960)]
assert names==["setVolume:","setOutputVolume:"]
scale=struct.unpack(">d",bytes.fromhex("3fd9db22d0e56042"))[0]
raw=0.3150142729282379
f32=lambda value:struct.unpack("<f",struct.pack("<f",value))[0]
print(json.dumps(dict(scope="current Mac ARM64 static master path, no AU runtime or hidden PCM tap",
    sha256=sha,master_slot=200,master_sender="0x1000ee4b4",scale=scale,exponent=1,
    mixer_setter="0x10047b638",selectors=names,brainwave_raw=raw,brainwave_output_volume=f32(raw*scale)),indent=2))
