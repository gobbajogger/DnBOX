"""Local disposable emulator-only seed. Never build or write an install package."""
from pathlib import Path
import fcntl
import argparse,hashlib, importlib.util,json,os,re,struct,subprocess,sys,zlib
P=argparse.ArgumentParser()
for k in ('workspace','lab','scratch'):P.add_argument(k,type=Path)
P.add_argument('--run',action='store_true'); P.add_argument('--models',nargs='+',default=['elite','pro'])
a=P.parse_args();w=a.workspace.resolve();lab=a.lab.resolve();o=a.scratch.resolve();o.mkdir(exist_ok=True,parents=True)
def sha(x):return hashlib.sha256(x).hexdigest()
emu=w/'full-gate-inputs/harness/fm1-live-ba83317a'
assert sha(emu.read_bytes())=='fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017'
assert o.is_relative_to(Path('/tmp')) and o!=Path('/tmp')
# Only the archived container SYNTHETIC seed extraction and elementary data
# transforms are used. No USB command, writer, SDK packager, UFW or FWSC creation.
readtool=w/'full-fw-next/tools/fm1_rescue.py'
spec=importlib.util.spec_from_file_location('read_only_archive',readtool)
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
enc,crc=module.enc,module.crc16
def u32(b,a):return struct.unpack_from('<I',b,a)[0]
def u16(b,a):return struct.unpack_from('<H',b,a)[0]
def sfc(a,key):
    b=bytearray()
    for i in range(0,len(a),32):
        p=bytearray(a[i:i+32]);enc(p,key^(i>>2));b+=p
    return b
seedid={'elite':'3cd2e16cc7119bf82e6274c3f46d2caad0dd576c0818dc43f9ab817a9dd1eee2',
        'pro':'e62b1dfe4a03c32abc57660dd228fe40ce575d0dd1f2b7cf8920cce82029c785'}
sealed={'elite': ('641db46ba1473548c0b1e769e454510436beea561205584efb46c643e6b358d2', 'bc76273cf14e3e7098f8696655a9b354d4c335d2b7e137e2295476c39ff16cbc', 'e724107e292367089769fbbee84d67dcb64a016ac92cd3999ca4e9fd731be608', '7ae644f8cda065e5db335a3678409a7bff08742c1882cce802d0ec647e793f0b', '75725eff074f17c5d82d8a03369dedf837c417ff2b41d3052e71b4ae7178547e'), 'pro': ('b41d8ab7be469675004946b2493172d7365cfa3e62f5af25d8a1f2529bed3cb2', '8b11b442130af205b2dc8875f527e2fbccc9de2f1f7eb6bff82c647c4af146fb', '20b7e399bb49b979b905a4ecd25a7ce6f8664c169d3349906eed9d79b2086fd4', '1aec5ad3637fba8e6d21d78e6f1c58c876e4fc6ed639f727c6240119afc43a81', 'ca91a9fdf2a373581014421202cb6ff6e16e26c004db8df50853556fa8432572')}
tests={}
for model in a.models:
    assert model in seedid
    fw=w/'out'/f'DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc'
    raw=fw.read_bytes();assert sha(raw)==seedid[model]
    base=module.package_flash_image(raw);assert len(base)==0x9d000
    # Decode the test template's public integrated JLFS metadata to locate its
    # model-preserving app extent; the read-only archive is never rewritten.
    isd_entry=bytearray(base[64:96]);enc(isd_entry)
    assert crc(isd_entry[2:])==u16(isd_entry,0)
    off=u32(isd_entry,4);isd=base[off:off+32]
    sm=sum(isd[:16])&255;sm=0xaa if sm>=0xe0 else 0x55 if sm<=0x10 else sm
    key=sum(1<<i for i in range(16) if (isd[16+i]^isd[15-i])<sm)
    region=sfc(base[0x4000:],key)
    extent=u32(region,8);assert 0x120<extent<=len(region)
    assert region[16:29].startswith(b'app_area_head') and crc(region[2:32])==u16(region,0)
    assert crc(region[32:extent])==u16(region,2)
    assert region[48:56].startswith(b'app.bin') and crc(region[34:64])==u16(region,32)
    appoffset,appslot=u32(region,36),u32(region,40)
    assert (appoffset,appslot)==(0x120,0x97400)
    assert crc(region[appoffset:appoffset+appslot])==u16(region,34)
    files=lab/model; elf=files/'never-install.elf'
    # Native sections assembled by LMA in the exact linked addresses.
    sym={}
    for line in subprocess.check_output(['nm','-S',elf],text=True).splitlines():
        p=line.split()
        if len(p) in (3,4):sym[p[-1]]=int(p[0],16)
    assert sym['_start']==0x02000120 and sym['cold_lab_start']==0x020af000 and sym['cold_lab_tail_start']==0x020fa000
    assert sha(elf.read_bytes())==sealed[model][0]
    app=bytearray((files/'text.bin').read_bytes())
    for n,link in [('ram_text','_rt_load'),('c1_text','_c1_load'),('data','_data_load')]:
        at=sym[link]-sym['_start'];assert len(app)<=at
        app+=bytes([255])*(at-len(app))+(files/(n+'.bin')).read_bytes()
    sections={k:int(n,16) for k,n in re.findall(
        r'^\.(\w+)\s+0x[a-f0-9]+\s+0x([a-f0-9]+)',(files/'lab.map').read_text(),re.M)}
    assert len(app)==sum(sections[k] for k in ('text','ram_text','c1_text','data'))
    assert 552000<len(app)<=552672
    xipmax=552672
    region[appoffset:appoffset+xipmax]=app+bytes([255])*(xipmax-len(app))
    struct.pack_into('<H',region,34,crc(region[appoffset:appoffset+appslot]))
    struct.pack_into('<H',region,32,crc(region[34:64]))
    struct.pack_into('<H',region,2,crc(region[32:extent]))
    struct.pack_into('<H',region,0,crc(region[2:32]))
    seed=bytearray(base+bytes([255])*(0x100000-len(base)))
    # Only app bytes/header in a synthetic file. Original template SPL, boot,
    # settings, cleanup area and all simulated real protected windows untouched.
    seed[0x4000:0x4000+appoffset+xipmax]=sfc(region,key)[:appoffset+xipmax]
    assert sfc(seed[0x4000:0x4000+appoffset+xipmax],key)[appoffset:appoffset+len(app)]==app
    xd=(files/'xdata.bin').read_bytes();assert len(xd)==7775
    pack=w/'s1-fix-data/s1.pack';s1=pack.read_bytes(); assert len(s1)==61208 and sha(s1)=='c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44'
    seed[0xad000:0xad000+len(xd)]=xd;seed[0xe5000:0xe5000+len(s1)]=s1
    # Synthetic occupied slot 1, outside every companion extent. A valid
    # independent FSMP header + opaque deterministic bytes, not owner data.
    occupied=bytearray(b'\xff'*4096)
    body=bytes((7+59*i)&255 for i in range(256))
    ctl=struct.pack('<4sHBB8s4I','FSMP'.encode(),1,1,0,b'LABSLOT1',len(body),zlib.crc32(body),0,0)
    zone=struct.pack('<5Ihh4B',0,len(body)*2,0,0,65536,60*16,0,0,0,127,0)
    occupied[:len(ctl)+len(zone)]=ctl+zone
    occupied[512:512+len(body)]=body
    seed[0xc2000:0xc3000]=occupied
    primary=(files/'lab_cold.bin').read_bytes()
    secondary=(files/'lab_fit.bin').read_bytes()
    heads=[]
    for blob,sym_name,digest,hd,magic in [
        (primary,'cold_lab_expected',sealed[model][1],sealed[model][3],b'COLD'),
        (secondary,'cold_lab_tail_expected',sealed[model][2],sealed[model][4],b'STB2')]:
        ofs=sym[sym_name]-sym['_start'];head=blob[:32];heads.append(head)
        assert sha(blob)==digest and app[ofs:ofs+32]==head and sha(head)==hd
        assert head[:4]==magic and u32(head,4)==2
        assert u32(head,8)==(1 if model=='elite' else 2)
        assert len(blob)==32+u32(head,12) and zlib.crc32(blob[32:])==u32(head,16)
    assert heads[0][20:]==heads[1][20:]
    assert len(primary)==13419==sections['lab_cold'] and len(secondary)==4096==sections['lab_fit']
    seed[0xb3000:0xb3000+len(primary)]=primary
    changed=secondary[:400]+bytes([secondary[400]^64])+secondary[401:]
    assert changed[:32]==secondary[:32] and zlib.crc32(changed[32:])!=u32(secondary,16)
    for mode,payload in [('valid',secondary)]+([('secondary-crc',changed)] if model=='elite' else []):
        loc=o/(model+'-'+mode);loc.mkdir(exist_ok=False)
        f=loc/'local-only.flash'
        full=bytearray(seed)
        full[0xfe000:0xff000]=payload
        f.write_bytes(full)
        # Verify untouched template and protected extents; only the designated
        # secondary sector changes in this local disposable file.
        assert full[:0x4000]==base[:0x4000] and full[0xc0000:0xc2000]==bytes([255])*0x2000
        assert full[0xf4000:0xfe000]==bytes([255])*0xa000 and full[0xff000:]==bytes([255])*0x1000
        tests[model,mode]=(fw,elf,sym,loc,sha(full),sha(app),sha(primary),sha(payload),sha(occupied))
print('Verified archived boot shims and local-only paired emulator seed hashes')
for (m,k),(_,_,_,_,seed,app,pri,sec,sample) in tests.items():print(m,k,seed,app,pri,sec,sample,flush=True)
if not a.run:sys.exit(0)
lock=open('/tmp/dnbox-gate-firmware.lock','a'); fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
results=[]
for (m,k),(fw,elf,syms,loc,seeddigest,appdigest,primdigest,secdigest,sampledigest) in tests.items():
    f=loc/'local-only.flash';pc=(loc/'stereo.s16le');ram=loc/'ram.bin';orig=f.read_bytes()
    env={x:y for x,y in os.environ.items() if not x.startswith('FM1_')}
    (loc/'work').mkdir(exist_ok=True)
    stimulus=(
        '1600:e0:+8,1700:SEQ,1740:SEQ^,1800:k5@127,1840:k5^,'
        '1900:k9@127,1940:k9^,2000:k12@127,2040:k12^,'
        '2110:k16@127,2150:k16^,2200:SEQ PLAY,2250:SEQ PLAY^,'
        '2400:m992464,2600:m892400,2700:m913064,3070:m813000'
        ) if k=='valid' else ''
    env.update(FM1_STRICT='1',FM1_NO_SKIP='1',FM1_BENCH_KEYS=stimulus,FM1_DUMPRAM=str(ram),FM1_DNBOX_PCM=str(pc))
    with (loc/'run.log').open('w') as log:
        subprocess.run([str(emu),'--fwsc',str(fw),'--mhz','360','--shot','3500' if k=='valid' else '2400',
                        '--lcd-only','--flash',str(f)],env=env,cwd=loc,stdout=log,stderr=subprocess.STDOUT,check=True)
    logtext=(loc/'run.log').read_text();lcd=loc/'work/lcd.ppm'
    assert f'lcd.ppm at {3500 if k=="valid" else 2400} ms' in logtext and 'skipped 0 cycles' in logtext
    mem=ram.read_bytes();rd=lambda n: u32(mem,syms[n]-0x01c00000)
    bad=mem[syms['cold_lab_bad']-0x01c00000]
    dbg=tuple(rd('x0x_dbg') for _ in [0])+struct.unpack_from('<12I',mem,syms['x0x_dbg']-0x01c00000+4)
    audio=pc.read_bytes() if pc.is_file() else b''
    protected=orig[:0x9d000]+orig[0xc0000:0xc2000]+orig[0xf4000:]
    final=f.read_bytes()
    assert protected==final[:0x9d000]+final[0xc0000:0xc2000]+final[0xf4000:]
    assert orig[0xb3000:0xb7000]==final[0xb3000:0xb7000]
    assert orig[0xfe000:0xff000]==final[0xfe000:0xff000]
    assert orig[0xc2000:0xc3000]==final[0xc2000:0xc3000]
    ok=k=='valid'
    if not ok:assert orig==final # failure never mutates the disposable flash
    scanned=mem[syms['usr_nz']-0x01c00000]
    assert scanned==(1 if ok else 0)
    assert bool(bad)!=ok and dbg[0]==0x44424731
    pass_audio=(dbg[1]>0 and rd('eng_scope_w')>0 and any(audio) and dbg[5]==0 and rd('audio_xruns')==0) if ok else (dbg[1]==0 and rd('eng_scope_w')==0 and not any(audio))
    res={'model':m,'expected_audio_and_execution_match':pass_audio,'audio_has_nonzero_pcm':any(audio),'variant':k,'cold_lab_bad':bad,'halves':dbg[1],'worst_us':dbg[2],
         'late':dbg[5],'xruns':rd('audio_xruns'),'scope_w':rd('eng_scope_w'),'lcd_sha256':sha(lcd.read_bytes()),
         'pcm_bytes':len(audio),'sample_slot_scanned_zones':scanned,'pcm_sha256':sha(audio),'seed_sha256':seeddigest,'app_sha256':appdigest,
         'primary_sha256':primdigest,'secondary_sha256':secdigest,'sample_sector_sha256':sampledigest,'full_flash_unchanged':orig==final,'elf_sha256':sha(elf.read_bytes())}
    (loc/'result.json').write_text(json.dumps(res,indent=2)+'\n')
    print(json.dumps(res),flush=True)
    results.append(res)
assert len(results)==3 and sum(v['variant']=='valid' for v in results)==2
assert all(v['expected_audio_and_execution_match'] for v in results)
negative=next(v for v in results if v['variant']!='valid')
assert negative['lcd_sha256']=='ba5965eb74ba948569a7fbaa2cc5cd4eeeeb19d00d4eed29c21773ac7da365ce'
assert all(v['lcd_sha256']!=negative['lcd_sha256'] for v in results if v['variant']=='valid')
