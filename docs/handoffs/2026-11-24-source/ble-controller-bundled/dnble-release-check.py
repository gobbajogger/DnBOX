import pathlib,sys,json,os,subprocess,struct,hashlib,fcntl,array
import add_ctl as c
from fm1_rescue import package_flash_image
w=pathlib.Path.cwd(); d=pathlib.Path(sys.argv[1]); emu=w/'full-gate-inputs/harness/fm1-live-ba83317a'
sha=lambda v:hashlib.sha256(v).hexdigest()
assert sha(emu.read_bytes())=='fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017'
cat=json.loads((w/'pages/versions.json').read_text())['versions'][0]
scene='1600:e0:+8,1700:SEQ,1740:SEQ^,1800:k5@127,1840:k5^,1900:k9@127,1940:k9^,2000:k12@127,2040:k12^,2110:k16@127,2150:k16^,2200:SEQ PLAY,2250:SEQ PLAY^'
with open('/tmp/dnbox-gate-firmware.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
 for model, v in cat['models'].items():
  p=w/'pages'/v['pkg'].split('DnBOX-nightly/')[1];raw=p.read_bytes(); assert len(raw)==v['size'] and sha(raw)==v['sha256']
  product,logical=c.logical_of(raw); x=c.controller_of(logical)
  assert product==v['product'] and x and len(x)==v['controller_size'] and sha(x)==v['controller_sha256']=='a334208b51f62bc7231ebb76a6de0dfa260033d7f7316e5b92053c540bc1d984' and c.ctl_problem(x) is None
  print(json.dumps(dict(model=model,product=product, fwsc_sha=sha(raw), ctl_sha=sha(x), ctl_size=len(x), header=struct.unpack_from('<IHH12I',x))),flush=True)
  for dur in (2500,3500):
   loc=d/f'{model}-{dur}';loc.mkdir()
   env={k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
   env.update(FM1_STRICT='1',FM1_NO_SKIP='1',FM1_BENCH_KEYS=scene,FM1_DNBOX_PCM=str(loc/'audio.pcm'),FM1_DUMPRAM=str(loc/'ram'))
   with (loc/'run.log').open('w') as fp:
    r=subprocess.run([str(emu),'--fwsc',str(p),'--mhz','360','--shot',str(dur),'--lcd-only','--flash',str(loc/'flash')],cwd=loc,env=env,stdout=fp,stderr=subprocess.STDOUT,timeout=120)
   ram=(loc/'ram').read_bytes() if (loc/'ram').is_file() else b''
   mb=[(hex(0x1c00000+i),struct.unpack_from('<20I',ram,i)) for i in range(0,len(ram)-80,4) if ram[i:i+8]==b'XBMX\x02\0\0\0']
   pcm=(loc/'audio.pcm').read_bytes() if (loc/'audio.pcm').is_file() else b''
   flash=(loc/'flash').read_bytes() if (loc/'flash').is_file() else b''
   prot=package_flash_image(raw)
   log=(loc/'run.log').read_text(errors='replace')
   dbg=[(hex(0x1c00000+i), struct.unpack_from('<13I',ram,i)) for i in range(0,len(ram)-52,4) if ram[i:i+4]==b'1GBD']
   print(json.dumps(dict(model=model,duration=dur,ret=r.returncode, strict_no_skip='skipped 0 cycles' in log,shot=f'lcd.ppm at {dur} ms' in log,ram_sha=sha(ram), mailbox=mb, dbg=dbg, pcm_sha=sha(pcm), pcm_size=len(pcm),pcm_peak=max(map(abs,array.array('h',pcm)),default=0),protected_same=prot==flash[:len(prot)],scene_sha=sha(scene.encode()))),flush=True)
