"""Project exact paired native laboratory ELFs into disposable emulator NOR only.
No package creation, hardware operation, connectivity, or physical file-device writes.
"""
import argparse, concurrent.futures, hashlib, importlib.util, json, os, re, stat, struct, subprocess, tempfile, zipfile, zlib
from pathlib import Path
START=0x02000120
MAX=0x86ee0
ELF_RAM=0x01c00000
FLASH_SIZE=0x100000
STOCK={
 'elite':'3cd2e16cc7119bf82e6274c3f46d2caad0dd576c0818dc43f9ab817a9dd1eee2',
 'pro':'e62b1dfe4a03c32abc57660dd228fe40ce575d0dd1f2b7cf8920cce82029c785'}
EMU='fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017'
SCENE='d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271'
ARCHIVE='01350ef744652ff7bc27501ddcd11037999c2f6f2bc8850ddaa2336ab6bd91cc'
def ensure(x,msg):
 if not x: raise ValueError(msg)
def regular(path):
 ensure(path.is_file() and stat.S_ISREG(path.stat().st_mode),f'not a regular input: {path}')
 return path.read_bytes()
def sha(b):return hashlib.sha256(b).hexdigest()
def u16(b,k):return struct.unpack_from('<H',b,k)[0]
def u32(b,k):return struct.unpack_from('<I',b,k)[0]
def symbols_and_sections(path):
 b=regular(path)
 ensure(b[:6]==b'\x7fELF\x01\x01','expected 32-bit little-endian native ELF')
 sym={}
 for line in subprocess.check_output(['readelf','-sW', path], text=True).splitlines():
  f=line.split()
  if len(f)>=8 and f[2].isdigit() and f[6]!='UND': sym[f[7]]=int(f[1],16)
 sec={}
 for name,typ,vma,off,n in re.findall(r'^\s*\[\s*\d+\]\s+(\.[\w.]+)\s+(\w+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',subprocess.check_output(['readelf','-SW',path],text=True),re.M):
  va,o,z=map(lambda x:int(x,16),(vma,off,n))
  ensure(typ=='NOBITS' or 0<=o<=len(b)-z,f'ELF section extent: {name}')
  sec[name]=(va,z, None if typ=='NOBITS' else b[o:o+z])
 return b,sym,sec
def section(sec,name,vma,limit):
 a,n,b=sec[name]
 ensure(a==vma and 0<n<=limit and b is not None,f'native region {name}')
 return b
def app_sections(sym,sec):
 ensure(sym['_start']==START,'start mismatch')
 ensure(sec['.pool'][0]==0x01c20000 and sec['.pool'][1]<=0x54000-8192,'POOL_MIN')
 txt=section(sec,'.text',START,MAX)
 img=bytearray(txt)
 ram=[]
 for n,label in [('ram_text','rt'),('c1_text','c1'),('data','data')]:
  v=sym[f'_{label}_start']; off=sym[f'_{label}_load']-START
  lo,hi=(ELF_RAM+0x400,ELF_RAM+0x1800) if n!='data' else (ELF_RAM+0x8000,ELF_RAM+0x20000)
  r=section(sec,'.'+n,v,hi-v)
  ensure(lo<=v<hi and off%4==0 and len(img)<=off<=MAX-len(r),f'load extent {n}')
  img.extend(b'\xff'*(off-len(img)));img.extend(r)
  if n!='data':ram.append((v-ELF_RAM,r))
 img.extend(b'\xff'*(-len(img)%4));img.extend(b'\x00\xff\xff\xff')
 ensure(500000<len(img)<=MAX,'app overlap with BLE')
 return bytes(img),ram
def loader(module,base,img):
 enc,crc=module.enc,module.crc16
 def stream(b,key):
  dst=bytearray()
  for i in range(0,len(b),32):
   x=bytearray(b[i:i+32]);enc(x,key^(i>>2));dst.extend(x)
  return dst
 e=bytearray(base[64:96]);enc(e)
 ensure(crc(e[2:])==u16(e,0),'stock ISD record CRC')
 at=u32(e,4)
 ensure(0<=at<=len(base)-32,'stock ISD record extent')
 isd=base[at:at+32]
 sm=sum(isd[:16])&255;sm=0xaa if sm>=0xe0 else 0x55 if sm<=0x10 else sm
 key=sum(1<<i for i in range(16) if (isd[16+i]^isd[15-i])<sm)
 r=stream(base[0x4000:],key)
 extent=u32(r,8);off,slot=u32(r,36),u32(r,40)
 def valid(r):
  return (r[16:29].startswith(b'app_area_head') and r[48:56].startswith(b'app.bin')
          and crc(r[2:32])==u16(r,0) and crc(r[32:extent])==u16(r,2)
          and crc(r[34:64])==u16(r,32) and crc(r[off:off+slot])==u16(r,34))
 ensure((off,slot)==(0x120,0x97400) and off<extent<=len(r) and valid(r),'stock app directory')
 r[off:off+MAX]=img+b'\xff'*(MAX-len(img))
 struct.pack_into('<H',r,34,crc(r[off:off+slot]))
 struct.pack_into('<H',r,32,crc(r[34:64]))
 struct.pack_into('<H',r,2,crc(r[32:extent]))
 struct.pack_into('<H',r,0,crc(r[2:32]))
 end=off+MAX
 encrypted=stream(r[:end],key)
 decoded=stream(encrypted+base[0x4000+end:],key)
 ensure(valid(decoded) and decoded[off:off+len(img)]==img,'candidate app directory')
 return base[:0x4000]+encrypted+base[0x4000+end:]
def project(module,model,pkg,elf,expected):
 b,sym,sec=symbols_and_sections(elf)
 ensure(sha(b)==expected,'native fit ELF SHA does not match '+model)
 raw=regular(pkg)
 ensure(sha(raw)==STOCK[model],'stock template SHA mismatch')
 base=module.package_flash_image(raw)
 ensure(len(base)==0x9d000,'stock flash extent')
 img,ram=app_sections(sym,sec)
 seed=bytearray(loader(module,base,img))
 seed.extend(b'\xff'*(FLASH_SIZE-len(seed)))
 companions={}
 expected_board={'elite':1,'pro':2}[model]
 guards=[]
 for name,magic,symbol,at,limit in (
     ('.lab_fdata',b'D9RD','cold_lab_expected',0xe5000,0xf000),
     ('.lab_slot3',b'D9T3','cold_slot3_expected',0xb3000,0x8000),
     ('.lab_fittab',b'D9FT','cold_fittab_expected',0xfe000,0x1000)):
  payload=section(sec,name,0x02000000+at-0x4000,limit)
  off=sym[symbol]-START
  ensure(0<=off<=len(img)-32 and payload[:4]==magic and u32(payload,4)==2 and
         payload[:32]==img[off:off+32] and u32(payload,8)==expected_board and
         u32(payload,12)==len(payload)-32 and u32(payload,16)==zlib.crc32(payload[32:]),f'{model} guarded {name}')
  guards.append(payload[20:32])
  companions[at]=payload
 ensure(len(set(guards))==1,'companion identity mismatch')
 xd=section(sec,'.xdata',0x020a9000,0x2000)
 n=u32(xd,4)
 ensure(xd[:4]==b'XXD1' and n<=len(xd)-16 and zlib.crc32(xd[16:16+n])==u32(xd,8)==u32(xd,12),'stock XDATA data CRC')
 if 'cold_xdata_tail_crc' in sym:
  start=sym['cold_xdata_tail_start']-0x020a9000
  end=sym['cold_xdata_tail_end']-0x020a9000
  off=sym['cold_xdata_tail_crc']-START
  ensure(16+n<=start<end==len(xd) and 0<=off<=len(img)-4 and zlib.crc32(xd[start:end])==u32(img,off),'XDATA extra tail CRC')
 else:
  ensure(n+16==len(xd),'unrecognized XDATA tail')
 companions[0xad000]=xd
 for at,data in companions.items():
  ensure(seed[at:at+len(data)]==b'\xff'*len(data),f'nonempty simulated region: {at:#x}')
  seed[at:at+len(data)]=data
 lim=0x4000+0x120+MAX
 ensure(seed[:0x4000]==base[:0x4000] and seed[lim:0x9d000]==base[lim:],'protected stock boot/BLE changed')
 return bytes(seed),sym,ram
def replay(w,board,offset,emu,pkg,events,projected):
 data,sym,expected_ram=projected
 d=w/f'{board}-offset-{offset}';d.mkdir()
 flash=d/'virtual-nor.bin';flash.write_bytes(data)
 keys=d/'scene.keys';keys.write_text(','.join(f'{t+max(offset,0)}:{k}' for t,k in events)+'\n')
 pcm=d/'stereo.s16le';ram=d/'ram.bin';duration=2000 if offset<0 else 8000
 env={k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
 env.update(FM1_STRICT='1',FM1_NO_SKIP='1',FM1_BENCH_KEYS='@'+str(keys),FM1_DNBOX_PCM=str(pcm),FM1_DUMPRAM=str(ram))
 (d/'work').mkdir()
 with (d/'run.log').open('w') as f:
  subprocess.run([str(emu),'--fwsc',str(pkg),'--mhz','360','--shot',str(duration),'--flash',str(flash)],env=env,cwd=d,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=900)
 log=(d/'run.log').read_text()
 ensure(f'panel.ppm at {duration} ms' in log and 'SMK-37 pads' in log and 'skipped 0 cycles' in log,'wrong scene execution')
 final=regular(flash)
 ensure(len(final)==len(data),'virtual flash changed size')
 changes=[i for i,(x,y) in enumerate(zip(final,data)) if x!=y]
 ensure(all(0xaf000<=x<0xaf010 for x in changes),'unexpected virtual flash mutation')
 r=regular(ram)
 for i,x in expected_ram:
  ensure(r[i:i+len(x)]==x,'candidate native executable SRAM did not match')
 def address(name):
  i=sym[name]-ELF_RAM
  ensure(0<=i<len(r),f'invalid RAM symbol: {name}')
  return i
 d0=address('x0x_dbg');dbg=struct.unpack_from('<13I',r,d0)
 safe=r[address('safe_mode')];guard=r[address('cold_lab_bad')]
 xr=u32(r,address('audio_xruns'))
 stack=r[address('_c1_ustack'):address('_c1_ustack_top')]
 ensure(len(stack)==4352,'Core 1 stack layout')
 mark=struct.pack('<I',0x43314d4b); free=next((i for i in range(0,4352,4) if stack[i:i+4]!=mark),4352)
 p=regular(pcm);peak=max(abs(n) for n, in struct.iter_unpack('<h',p))
 halves=[p[i:i+2048] for i in range(0,len(p),2048)]
 repeats=[i for i in range(2,len(halves)) if any(halves[i]) and halves[i]==halves[i-2]]
 playing=r[address('seq')+8]
 ensure(dbg[0]==0x44424731 and dbg[1]>0 and safe==guard==0 and free>=256,'boot/stack/native guard witness')
 if offset>=0:ensure(peak>0 and playing==1,'silent/nonplaying scene')
 d=dict(board=board,offset=offset,max_us=dbg[2],rendered_halves=dbg[1],late=dbg[5],xruns=xr,stack_untouched=free,pcm_peak=peak,transport=playing,scene_sha=sha(regular(keys)),virtual_changes=len(changes),active_pcm_repeats=repeats,target_pass=offset>=0 and dbg[2]<10909 and dbg[5]==xr==0 and not repeats)
 (flash.parent/'result.json').write_text(json.dumps(d,indent=2)+'\n')
 return d
def main():
 p=argparse.ArgumentParser()
 p.add_argument('--workspace',type=Path,required=True)
 p.add_argument('--archive',type=Path,required=True)
 for m in STOCK:
  p.add_argument('--'+m,type=Path,required=True)
  p.add_argument('--'+m+'-sha',required=True)
 p.add_argument('--offsets',default='-1,0,1,2,3,4,5')
 p.add_argument('--jobs',type=int,default=2)
 p.add_argument('--run',action='store_true')
 a=p.parse_args();w0=a.workspace.resolve()
 ensure(regular(a.archive) and sha(regular(a.archive))==ARCHIVE,'harness archive SHA')
 offsets=[int(x) for x in a.offsets.split(',')]
 ensure(len(set(offsets))==len(offsets) and all(-1<=n<=5 for n in offsets) and 1<=a.jobs<=4,'replay bounds')
 spec=importlib.util.spec_from_file_location('stock_template_reader',w0/'fw/tools/fm1_rescue.py')
 mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
 scene=regular(w0/'fw/experiments/sound-hardware-v1/strict-timing/heavy.keys')
 ensure(sha(scene)==SCENE,'scene SHA')
 events=[(int(t),k) for t,k in (x.split(':',1) for x in scene.decode().strip().split(','))]
 ensure(len(events)==922,'scene event count')
 pkgs={m:w0/'out'/f'DnBOX-0.0.27-SMK37-{m}-73749f55-test.fwsc' for m in STOCK}
 projected={m:project(mod,m,pkgs[m],getattr(a,m),getattr(a,m+'_sha')) for m in STOCK}
 w=Path(tempfile.mkdtemp(prefix='dnbox-emulator-only-',dir='/tmp')).resolve()
 print(w, flush=True)
 for m in STOCK: (w/f'{m}-virtual-nor.bin').write_bytes(projected[m][0])
 if not a.run:return
 with zipfile.ZipFile(a.archive) as z:b=z.read('fm1-live-ba83317a')
 ensure(sha(b)==EMU,'emulator SHA')
 emu=w/'fm1-live-ba83317a';emu.write_bytes(b);emu.chmod(0o700)
 with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
  for d in pool.map(lambda x:replay(w,*x,emu,pkgs[x[0]],events,projected[x[0]]),[(m,o) for o in offsets for m in STOCK]):
   print(json.dumps(d),flush=True)
if __name__=='__main__':main()
