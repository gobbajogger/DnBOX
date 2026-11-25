"""Native cold-paired emulator only. No device or installer/upgrade artifact."""
from pathlib import Path
import fcntl,hashlib,json,os,re,struct,subprocess,sys,zlib
root,w,seed=map(lambda p:Path(p).resolve(),sys.argv[1:4]); model,v=sys.argv[4:6]; v=int(v)
assert model in ('elite','pro') and v in (0,1)
elf=w/'src/gatelab'/model/'never-install.elf'
fw=root/'out'/f'DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc'
emu=root/'full-gate-inputs/harness/fm1-live-ba83317a'
def sha(x):return hashlib.sha256(x).hexdigest()
assert sha(emu.read_bytes())=='fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017'
assert sha(elf.read_bytes())==(w/f'gate-{model}.log').read_text().split('ELF ')[-1].strip()
off={k:int(v) for k,v in re.findall(r'^@([a-zA-Z]+) = constant i32 (\d+)',(w/'gate-layout.ll').read_text(),re.M)}
sy={}
for ln in subprocess.check_output(['nm','-S',elf],text=True,stderr=subprocess.DEVNULL).splitlines():
    p=ln.split()
    if len(p)==4:sy[p[-1]]=tuple(int(x,16) for x in p[:2])
assert sy['original_bass'][1]==2*off['partBytes'],(sy['original_bass'],off)
screen=w/f'native-{model}-{v}';screen.mkdir();(screen/'work').mkdir()
s=bytearray((seed/f'{model}-valid/local-only.flash').read_bytes())
assert s[0xaf000:0xaf010]==b'\xff'*16
s[0xaf000:0xaf010]=b'SMK37DATAX0X'+bytes(4)
d=(w/'gate-project.bin').read_bytes();seen={}
while d:
    j,n=struct.unpack_from('<II',d);body,d=d[8:8+n],d[8+n:]
    assert j not in seen and j<7 and n<=3840;seen[j]=body
    at=0x9d000+8192*j;hdr=struct.pack('<IHHIIIII',0x30583058,j,0,1,n,zlib.crc32(body),0xffffffff,0xffffffff)
    assert s[at:at+8192]==b'\xff'*8192
    s[at:at+32]=hdr+struct.pack('<I',zlib.crc32(hdr));s[at+256:at+256+n]=body
assert len(seen[1])==2716
flash=screen/'fresh.flash';flash.write_bytes(s)
# Default part is 909; ALGO moves through 808 then Bass A and B.
keys='2350:e2:+4,2600:e2:+4,'+('2900:e2:-2,' if v else '')
keys+='3100:SEQ,3150:SEQ^,3310:e1:+4,3520:e1:+4,3730:e1:+4,3820:SEQ REC,3880:SEQ REC^,'
keys+='3990:k14@127,4130:e6:-2,4310:e6:-2'
env={k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
env.update(FM1_STRICT='1',FM1_NO_SKIP='1',FM1_BENCH_KEYS=keys,
           FM1_DUMPRAM=str(screen/'ram.bin'), FM1_DNBOX_PCM=str(screen/'audio.pcm'))
lock=open('/tmp/dnbox-gate-firmware.lock','a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
with (screen/'native.log').open('w') as f:
    subprocess.run([str(emu),'--fwsc',str(fw),'--mhz','360','--shot','4570','--lcd-only','--flash',str(flash)],cwd=screen,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
log=(screen/'native.log').read_text(); assert 'lcd.ppm at 4570 ms' in log and 'skipped 0 cycles' in log
mem=(screen/'ram.bin').read_bytes();u32=lambda at:struct.unpack_from('<I',mem,at)[0]
p0=sy['original_bass'][0]-0x1c00000
record=[]
for i in range(2):
    t=p0+off['partBytes']*i
    record.append({k:(u32 if k in ('gateTicks','latchTicks','beatTick') else lambda o:mem[o])(t+off[k])
                   for k in ('gateCfg','gateTicks','latchTicks','beatTick','gateOpen','noteCount','arpKind','arpDiv')})
q=sy['seq'][0]-0x1c00000+off['drumCh']
assert tuple(mem[q+2:q+4])==(1,2)
ui=sy['ui'][0]-0x1c00000
audio=(screen/'audio.pcm').read_bytes()
u=sy['x0x_dbg'][0]-0x1c00000
fin=flash.read_bytes()
for a,b in [(0,0x9d000),(0xad000,0xaf000),(0xc0000,0xc3000),(0xe5000,0x100000)]:assert fin[a:b]==s[a:b]
o={'model':model,'owner':v,'compiled_ram_offsets':off,'keys':keys,'ui_first4':list(mem[ui:ui+4]),'state':record,
   'halves':u32(u+4),'max_us':u32(u+8),'raw_late':u32(u+20),'audio_has_nonzero_pcm':any(audio),
   'elf_sha256':sha(elf.read_bytes()),'flash_unchanged':fin==s,'lcd_sha256':sha((screen/'work/lcd.ppm').read_bytes())}
(screen/'result.json').write_text(json.dumps(o,indent=2)+'\n')
print(json.dumps(o),flush=True)
p,q=record[v],record[1-v]
assert p['arpKind']==q['arpKind']==3 and p['arpDiv']==q['arpDiv']==15
assert 1<=p['gateCfg']<50 and q['gateCfg']==50,(p,q)
assert p['gateOpen'] and p['noteCount'] and 0<p['beatTick']<p['gateTicks']<p['latchTicks']
assert p['latchTicks']==q['gateTicks'] and mem[ui]==8 and mem[ui+1]==v+2
assert q['noteCount']==q['latchTicks']==0 and any(audio)
print(f"PASS native K3 50→{p['gateCfg']}: already-playing old deadline retained, peer unchanged")
