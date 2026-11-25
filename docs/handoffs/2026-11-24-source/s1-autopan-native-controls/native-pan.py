"""Audit real SMK inputs on exact linked firmware in the disposable emulator."""
import fcntl,hashlib,json,os,re,struct,subprocess,sys,tempfile,zlib
from pathlib import Path
root,w,seed,objs=map(lambda p:Path(p).resolve(),sys.argv[1:5])
model=sys.argv[5];shot=int(sys.argv[6]);keys=sys.argv[7]
assert model in ("pro","elite") and shot<=10000
elf=w/"src/labout"/model/"never-install.elf"
fw=root/"out"/f"DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc"
emu=root/"full-gate-inputs/harness/fm1-live-ba83317a"
def sha(blob):return hashlib.sha256(blob).hexdigest()
assert sha(emu.read_bytes())=="fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017"
assert sha(objs.read_bytes())=="53101e99ef84e39ff050f20eaaa32ed2ffa90ff730b8bd0fcdc0a4280ac15f6d"
off={k:int(v) for k,v in re.findall(r"^@(\w+) = constant i32 (\d+)",(w/"pan-layout.ll").read_text(),re.M)}
sy={}
for row in subprocess.check_output(["nm","-S",elf],text=True,stderr=subprocess.DEVNULL).splitlines():
    p=row.split()
    if len(p)==4:sy[p[-1]]=tuple(int(x,16) for x in p[:2])
assert sy["original_bass"][1]==2*off["partBytes"] and sy["proj"][1]==off["setBytes"]
scratch=Path(tempfile.mkdtemp(prefix=f"pan-{model}-{sha(keys.encode())[:12]}-{shot}-",dir=w));(scratch/"work").mkdir(exist_ok=True)
s=bytearray((seed/f"{model}-valid/local-only.flash").read_bytes()); assert s[0xaf000:0xaf010]==b"\xff"*16
s[0xaf000:0xaf010]=b"SMK37DATAX0X"+bytes(4)
d=objs.read_bytes()
records={}
for j in range(6):
    n=int.from_bytes(d[:4],"little");body,d=d[4:4+n],d[4+n:];assert 0<n<=3840 and n==len(body)
    records[j]=body
    at=0x9d000+8192*j;h=struct.pack("<IHHIIIII",0x30583058,j,0,1,n,zlib.crc32(body),0xffffffff,0xffffffff)
    assert s[at:at+8192]==b"\xff"*8192;s[at:at+32]=h+struct.pack("<I",zlib.crc32(h));s[at+256:at+256+n]=body
assert not d and len(records[1])==2716 and records[1][:4]==b"V6JX"
flash=scratch/"fresh.flash";flash.write_bytes(s)
e={k:v for k,v in os.environ.items() if not k.startswith("FM1_")}
e.update(FM1_STRICT="1",FM1_NO_SKIP="1",FM1_BENCH_KEYS=keys,FM1_DUMPRAM=str(scratch/"ram.bin"),FM1_DNBOX_PCM=str(scratch/"audio.pcm"))
lock=open("/tmp/dnbox-gate-firmware.lock","a");fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
with (scratch/"native.log").open("w") as f:
    subprocess.run([str(emu),"--fwsc",str(fw),"--mhz","360","--shot",str(shot),"--lcd-only","--flash",str(flash)],cwd=scratch,env=e,stdout=f,stderr=subprocess.STDOUT,check=True)
log=(scratch/"native.log").read_text();assert f"lcd.ppm at {shot} ms" in log and "skipped 0 cycles" in log
mem=(scratch/"ram.bin").read_bytes()
address=lambda x:sy[x][0]-0x1c00000
word=lambda u:struct.unpack_from("<I",mem,u)[0]
fp=lambda u:struct.unpack_from("<f",mem,u)[0]
p0=address("original_bass");j0=address("proj");ui=address("ui");pp=address("pg")
values=[]
for i in range(2):
    p=p0+i*off["partBytes"];b=j0+off[f"own{i}"]
    values.append(dict(kind=mem[p+off["kind"]],raw_amount=mem[b+21],raw_rate=fp(b+22),applied_amount=fp(p+off["amount"]),applied_rate=fp(p+off["rate"]),phase=fp(p+off["phase"]),authored=mem[p+off["authored"]]))
snd=(scratch/"audio.pcm").read_bytes();dbg=address("x0x_dbg");final=flash.read_bytes()
assert s[:0x9d000]==final[:0x9d000] and s[0xb3000:]==final[0xb3000:]
r=dict(model=model,shot=shot,keys=keys,elf=sha(elf.read_bytes()),revision=mem[j0+off["rev"]],values=values,flash_unchanged=s==final,ui=list(mem[ui:ui+48]),page_titles=list(mem[pp+384:pp+408]),page_count=mem[pp+420],nonzero_pcm=any(snd),pcm_sha=sha(snd),native_halves=word(dbg+4),native_late=word(dbg+20),native_peak=word(dbg+8))
(scratch/"case.json").write_text(json.dumps(r,indent=2)+"\n")
print(json.dumps(r))
assert r["revision"]==0x70 and all(v["authored"] for v in values)
if len(sys.argv)>8:
    o=int(sys.argv[8]);assert o in (0,1)
    own,peer=values[o],values[1-o]
    initial=(32,48); initial_rate=(.25,.5)
    assert mem[ui]==1 and mem[ui+1]==2+o and mem[ui+13+o]==17
    assert r["page_titles"][17]==69 and r["flash_unchanged"] and r["nonzero_pcm"]
    assert own["raw_amount"]>initial[o] and own["raw_rate"]>initial_rate[o]
    assert peer["raw_amount"]==initial[1-o] and peer["raw_rate"]==initial_rate[1-o]
    assert abs(own["applied_amount"]-own["raw_amount"]/127)<1e-7 and own["applied_rate"]==own["raw_rate"]
    print(f"PASS exact {model} owner {o}: real K1/K2 edit Amount/Rate and readback; peer and flash unchanged. Timing not qualified.")
