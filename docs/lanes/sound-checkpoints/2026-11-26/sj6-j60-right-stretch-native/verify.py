from pathlib import Path
import hashlib,re,json,subprocess,sys
root=Path(__file__).resolve().parent
expected={
 'warm':{
 'elite':'6e7878f91f4c2f4d4fe5a8c3112d69845406a9a644a1c31dfa42a121dad01a33',
 'pro':'5937ef79add726e8fb8d5a493324b3401a136e81d64c8cb0611999ed505d48c5'},
 'independent':{
 'elite':'1c4c90527128bf36b5bab6fcd895514e70658fa75521b218758d448f5f9d4d5f',
 'pro':'05ef2b6f5f096f89b62053b65b189107fb171070170e20e3ebb3eff72fb15900'}}
want=sys.argv[1]
res={}
for m in ('elite','pro'):
 p=root/'out'/(('warm-' if want=='warm' else '')+m)
 elfs=[p/str(i)/'never-install.elf' for i in (0,1)]
 digs=[hashlib.sha256(f.read_bytes()).hexdigest() for f in elfs]
 assert digs[0]==expected[want][m]
 def symbols(e):
  s=subprocess.check_output(['readelf','-sW',str(e)],text=True)
  out={}
  for n in ('juno6_j60_process','_pool_start','_pool_end','_xpool_start','_xpool_end'):
   t=re.search(r'^\s*\d+:\s*([0-9a-f]+)\s+(\d+)\s+.*?\s'+re.escape(n)+'$',s,re.M)
   assert t is not None,n
   out[n]=[hex(int(t[1],16)),int(t[2])]
  return out
 z=[symbols(e) for e in elfs]
 assert {k:v for k,v in z[0].items() if k.startswith('_')}=={k:v for k,v in z[1].items() if k.startswith('_')}
 log=(root/('warm.log' if want=='warm' else (m+'.log'))).read_text()
 lines=re.findall(r'guarded '+m+r' XIP (\d+) free (\d+) pool usable (\d+) read-only FDATA (\d+) diagnostic slot3 span (\d+) slot3 free (\d+) read-only FITTAB (\d+) header [\da-f]+ ELF ([\da-f]+)',log)
 assert len(lines)==2 and [a[-1] for a in lines]==digs
 stock,change=[list(map(int,a[:-1])) for a in lines]
 assert stock[2:]==change[2:] and change[1]>=0
 assert change[0]-stock[0]==stock[1]-change[1]
 res[m]={'off_elf_exact_pin':True,'elf_sha':digs,'off_on_app_used':[stock[0],change[0]],'off_on_app_free':[stock[1],change[1]],
         'app_delta_bytes':change[0]-stock[0],'pool_extra_above_8192_bytes':stock[2],
         'fdata_slot3_slot3_free_fittab_sizes_unchanged':stock[3:], 'native_symbols_off_on':z}
print(json.dumps({'donor':want,'results':res},indent=2))
