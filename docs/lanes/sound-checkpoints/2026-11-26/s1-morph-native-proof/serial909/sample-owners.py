from pathlib import Path
import json,struct,subprocess,sys
e,r,layout=sys.argv[1:]; d=Path(r).read_bytes()
def tbl(rows,field):
 out={}
 for row in rows:
  c=row.split()
  if len(c)>=8 and c[6]!='UND':
   try:out[c[7]]=int(c[field],16 if field==1 else 10)
   except ValueError:pass
 return out
sym=tbl(subprocess.check_output(['readelf','-sW',e],text=True).splitlines(),1)
t=tbl(Path(layout).read_text().splitlines(),2)
b=sym['d909']-0x01c00000+t['off_smp']-1
voices={}
for i,name in enumerate(('CH','OH','CR','RD')):
 start=b+i*t['size_d9_smp_t']
 voices[name]={k:struct.unpack_from('<' + ('i' if k in ('playing','mute') else 'I'),d,start+t['off_'+k]-1)[0]
             for k in ('adpcm','playing','mute','pos','len','frac','inc','incf','huff_bit')}
print(json.dumps(voices,sort_keys=True))
