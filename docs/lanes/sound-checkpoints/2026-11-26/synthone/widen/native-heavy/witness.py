import hashlib,json,re,struct,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent
sys.path.insert(0,str(here.parent/'restored-harness-inspect/helpers'))
from guarded_replay import ELF_RAM,symbols_and_sections,ensure
elf,folder,seed=map(Path,sys.argv[1:])
_,symbols,_=symbols_and_sections(elf)
r=(folder/'ram.bin').read_bytes()
log=(folder/'run.log').read_text()
report=subprocess.check_output(['readelf','-sW',elf],text=True)
arm=re.search(r'^\s*\d+:\s+([0-9a-fA-F]+)\s+(\d+) OBJECT .*\boriginal_bass$',report,re.M)
span,target,position,ptr,embedded=map(int,re.findall(r'^\s*\.long\s+(\d+)',(here/'widen-offsets.s').read_text(),re.M))
assert arm and span*2==int(arm[2])
base=int(arm[1],16)-ELF_RAM
assert 0<=base<=len(r)-span*2
value=struct.unpack_from('<f',r,base+target)[0]
pos=struct.unpack_from('<I',r,base+position)[0]
history=struct.unpack_from('<I',r,base+ptr)[0]-ELF_RAM
assert 0<=history<=len(r)-4410 and history==base+embedded
encoded=r[history:history+4410]
nonzero=sum(x!=0 for x, in struct.iter_unpack('<H',encoded))
delta=[i for i,(a,b) in enumerate(zip(seed.read_bytes(),(folder/'disposable.bin').read_bytes())) if a!=b]
assert all(0xaf000<=i<0xaf010 for i in delta) and 'flash: 0 erases / programs' in log
abi=symbols['_c1_ustack']-ELF_RAM
stack=struct.unpack_from('<1088I',r,abi)
free=4*next((i for i,x in enumerate(stack) if x!=0x43314d4b),1088)
owner=json.loads((folder/'owner.json').read_text())
print(json.dumps(dict(target=value,position=pos,embedded_history_pointer=True,history_bytes=len(encoded),nonzero_samples=nonzero,hash_history=hashlib.sha256(encoded).hexdigest(),nor_changes=len(delta),c1_stack_free_bytes=free,owner=owner)),flush=True)
assert 0<value<=1 and 0<=pos<2205 and free>=256 and owner['count']>=4 and all(x['bank']==2 and x['status']==1 and x['cutoff']!=16383 and x['bankB']==0 and x['statusB']==1 and x['arp']==1 for x in owner['owners'])
