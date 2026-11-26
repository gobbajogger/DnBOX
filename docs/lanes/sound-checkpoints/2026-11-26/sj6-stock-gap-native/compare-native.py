from pathlib import Path
import re,json
d=Path(__file__).resolve().parent
base=json.loads((d.parent/'sj6-j60-right-native/warm.json').read_text())['results']
rows={}
for m in ('elite','pro'):
 modes={}
 for s,k in (('off','-off'),('on','')):
  t=(d/f'native-gaps-{m}{k}.receipt').read_text()
  pat=fr'guarded {m} XIP (\d+) free (\d+) pool usable (\d+) read-only FDATA (\d+) diagnostic slot3 span (\d+) slot3 free (\d+) read-only FITTAB (\d+) header ([0-9a-f]+) ELF ([0-9a-f]+)'
  q=re.findall(pat,t);assert len(q)==1,q
  used,free,pool,fdata,slot,slotfree,fittab=map(int,q[0][:7])
  assert [fdata,slot,slotfree,fittab]==base[m]['fdata_slot3_slot3_free_fittab_sizes_unchanged']
  addr={}
  for k in ('_pool_start','_pool_end','_xpool_start','_xpool_end','original_bass'):
   a=re.findall(rf'^{k} ([\da-f]+) (\d+)$',t,re.M);assert len(a)==2 and a[0]==a[1],(m,s,k,a)
   addr[k]=[a[0][0],int(a[0][1])]
  modes[s]=dict(used=used,free=free,pool_over_8192=pool,effective_unreserved_pool=pool-512,sha=q[0][-1],extent=[fdata,slot,slotfree,fittab],symbols=addr)
 off,on=modes['off'],modes['on']
 assert off['extent']==on['extent'] and off['symbols']==on['symbols'] and off['pool_over_8192']==on['pool_over_8192']==564
 rows[m]=dict(paired=modes,same_path_layout_app_delta=on['used']-off['used'],staging_offset_vs_pinned_prior=off['used']-base[m]['off_on_app_used'][1])
print(json.dumps(rows,indent=2))
