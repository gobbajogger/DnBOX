from pathlib import Path
import json,struct
def get(k):
 assert 'skipped 0 cycles' in Path(k+'.log').read_text()
 ram=Path(k+'.ram').read_bytes(); r=struct.unpack_from('<25I',ram,0x10000)
 n,=struct.unpack_from('<I',ram,0x20000); assert 2000<n<16384
 (payload:=globals().setdefault('payload',{}))[k]=(n,ram[0x20004:0x20004+(n+2048)*8])
 assert r[0]==0x53413146
 return list(zip(r[1::3],r[2::3],r[3::3]))
aa,bb=get('base'),get('candidate')
assert payload['base']==payload['candidate']; res=[]
for label,a,b in zip([str(i) for i in range(8)],aa,bb):
 assert a[1:]==b[1:]
 assert a[2] != 1307450821
 assert len(set(r[2] for r in aa))>=4
 res.append(dict(mode=label,base_ticks=a[0],new_ticks=b[0],saved_pct=(a[0]-b[0])*100/a[0],phase=a[1],pcm_words_fnv32=a[2]))
print(json.dumps(dict(armed_code_and_table_size_B=[Path(k+'.bin').stat().st_size for k in ('base','candidate')],N_samples_per_case=128,directly_compared_bytes=len(payload['base'][1]),owner_bytes_per_span=payload['base'][0],results=res),indent=2))
