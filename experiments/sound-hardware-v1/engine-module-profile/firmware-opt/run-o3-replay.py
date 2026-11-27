from pathlib import Path
import concurrent.futures, hashlib, importlib.util, json, struct, subprocess, tempfile, zipfile
import guarded_replay as g
r=Path.cwd(); d=r/'s1-focus-o3'; a=d/'tree/joint-diagnostic-o3'
spec=importlib.util.spec_from_file_location('stock_template_reader',r/'fw/tools/fm1_rescue.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
scene=g.regular(r/'fw/experiments/sound-hardware-v1/strict-timing/heavy.keys')
assert g.sha(scene)==g.SCENE
ev=[(int(t),k) for t,k in (e.split(':',1) for e in scene.decode().strip().split(','))]
assert len(ev)==922
w=Path(tempfile.mkdtemp(prefix='dnbox-emulator-only-',dir='/tmp'))
archive=r/'restored-harness-inspect/dot-gate-harness.zip'
assert g.sha(g.regular(archive))==g.ARCHIVE
with zipfile.ZipFile(archive) as z: b=z.read('fm1-live-ba83317a')
assert g.sha(b)==g.EMU
emu=w/'fm1-live-ba83317a';emu.write_bytes(b);emu.chmod(0o700)
def run(m):
    p=d/(m+'-o3.log'); line=next(l for l in p.read_text().splitlines() if l.startswith('guarded '+m+' XIP '))
    pin=line.split()[-1]
    elf=a/m/'never-install.elf';pkg=r/'out'/f'DnBOX-0.0.27-SMK37-{m}-73749f55-test.fwsc'
    nor,sym,ram=g.project(mod,m,pkg,elf,pin)
    pure=w/f'{m}-virtual-nor.bin';pure.write_bytes(nor)
    cold=g.replay(w,m,-1,emu,pkg,ev,(nor,sym,ram))
    assert cold['late']==0 and cold['xruns']==0
    seed=w/f'{m}-s1-arp.bin'
    cmd=['python3',str(r/'room-native-recovery/seed.py'),str(pure),str(elf),str(w/f'{m}-offset--1/ram.bin'),str(r/'remote-typed-load/tree/build/smk37-elite/x0x.elf'),str(r/'remote-typed-load/loaded-smoke-elite/ram.bin'),str(r/'room-native-recovery/fixture'),str(seed)]
    receipt=json.loads(subprocess.check_output(cmd,env={'PYTHONPATH':str(d),'PATH':'/usr/bin:/bin'}))
    raw=g.regular(seed);assert receipt['seed_sha256']==g.sha(raw)
    heavy=g.replay(w,m,0,emu,pkg,ev,(raw,sym,ram))
    ramfile=g.regular(w/f'{m}-offset-0/ram.bin')
    at=sym['s1_stage_worst']-g.ELF_RAM
    slots=struct.unpack_from('<20I',ramfile,at)
    assert slots[17]==heavy['max_us']
    jobnames=('909','808','A','B','BREAK')
    unit=lambda x: round(x/24.0,2)
    stages={'core0_jobs':dict(zip(jobnames,map(unit,slots[:5]))),'core1_jobs':dict(zip(jobnames,map(unit,slots[5:10]))),
            'jobs_wall':unit(slots[10]),'post_mix':unit(slots[11]),'tail_and_sub_remainder':unit(slots[12]-slots[10]-slots[11]),
            'outside_render_sub':round(heavy['max_us']-unit(slots[12]),2),'sub_calls':slots[13],
            'known_stage_wait_core0':unit(slots[14]),'known_stage_wait_core1':unit(slots[15]),
            'plans':[i for i in range(32) if slots[16]>>i&1],'half':slots[18],'prior_late':slots[19]}
    count,=struct.unpack_from('<I',ramfile,sym['lab_s1_voice_entries']-g.ELF_RAM)
    assert count>0,(m,heavy)
    return {'cold':cold,'heavy':heavy,'seed':receipt,'native':line,'worst_stage_us':stages,'confirmed_s1_voice_entries':count}
print(w,flush=True)
with concurrent.futures.ThreadPoolExecutor(2) as pool:
    for record in pool.map(run,('elite','pro')): print(json.dumps(record),flush=True)
