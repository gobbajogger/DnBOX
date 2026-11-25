"""Read settled SJ6 delay state from model-matched virtual emulator RAM."""
from pathlib import Path
import json,math,re,struct,subprocess,sys
elf,ram,offsets=map(Path,sys.argv[1:4])
symbol=re.search(r'^\s*\d+:\s+([0-9A-Fa-f]+)\s+(\d+) OBJECT .*\boriginal_bass$',subprocess.check_output(['readelf','-sW',elf],text=True),re.M)
assert symbol, 'original_bass missing'
span,*active=map(int,re.findall(r'^\s*\.long\s+(\d+)',offsets.read_text(),re.M))
assert int(symbol[2])==span*2
m=ram.read_bytes()
base=int(symbol[1],16)-0x01c00000+span
assert span<=base and base+span<=len(m)
names=['tap','current','fb','wet','inv_stride','stride','filled','mix','refused','sync','enabled']
k={key:struct.unpack_from('<'+t,m,base+i)[0] for key,t,i in zip(names,'IffffHHBBBB',active)}
assert .89<k['wet']<.91 and math.isclose(k['fb'],.99*64/127,rel_tol=1e-5)
assert (k['mix'],k['refused'],k['sync'],k['enabled'])==(127,0,1,1)
assert k['stride']>1 and k['filled']>=512
b=k.pop('tap')-0x01c00000
assert 0<=b and b+4096<=len(m)
words=struct.unpack_from('<2048h',m,b)
k['history_frames_loaded']=k['filled']
k['settled_tap_audio_frames']=k['current']*k['stride']
k['peak_abs_in_ring']=max(map(abs,words))
assert k['peak_abs_in_ring']>0
print(json.dumps(k))
