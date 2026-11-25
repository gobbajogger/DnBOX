import hashlib,json,math,re,struct,subprocess,sys
from pathlib import Path
elf, ram, offsets, phase = sys.argv[1:5]
symbol=re.search(r'^\s*\d+:\s+([0-9A-Fa-f]+)\s+(\d+) OBJECT .*\boriginal_bass$',subprocess.check_output(['readelf','-sW',elf],text=True),re.M)
assert symbol
span,*active=map(int,re.findall(r'^\s*\.long\s+(\d+)',Path(offsets).read_text(),re.M))
assert int(symbol[2])==span*2
m=Path(ram).read_bytes()
base=int(symbol[1],16)-0x01c00000+span
names=['tap','current','fb','wet','inv_stride','stride','filled','mix','refused','sync','enabled']
k={key:struct.unpack_from('<'+t,m,base+i)[0] for key,t,i in zip(names,'IffffHHBBBB',active)}
assert math.isclose(k['fb'],.99*63/127,rel_tol=1e-5)
assert k['sync']==1 and k['refused']==0 and k['stride']==16 and k['filled']==1024
assert (k['mix'],k['enabled'])==((0,0) if phase=='dry' else (127,1))
assert (k['wet']==0) if phase=='dry' else .89<k['wet']<.91
b=k.pop('tap')-0x01c00000
assert 0<=b and b+4096<=len(m)
words=struct.unpack_from('<2048h',m,b)
k['settled_tap_audio_frames']=k['current']*k['stride']
k["ring_sha256"]=hashlib.sha256(m[b:b+4096]).hexdigest()
k['nonzero_history_words']=sum(v!=0 for v in words)
k['peak_abs_in_ring']=max(map(abs,words))
assert k['settled_tap_audio_frames']==(11025 if phase=='early' else 10584)
if phase!='dry': assert k['peak_abs_in_ring']>0
print(json.dumps(k))
