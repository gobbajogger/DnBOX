import subprocess, sys,re,hashlib
from pathlib import Path
for f in map(Path,sys.argv[1:]):
 print(hashlib.sha256(f.read_bytes()).hexdigest(),f)
 s=subprocess.check_output(['readelf','-sW',str(f)],text=True)
 for k in ('_pool_start','_pool_end','_xpool_start','_xpool_end','original_bass'):
   t=re.search(r'^\s*\d+:\s*([0-9a-f]+)\s+(\d+)\s+.*?\s'+k+'$',s,re.M)
   assert t,k
   print(k,t[1],t[2])
