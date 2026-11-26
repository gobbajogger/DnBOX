"""Read actual state/PCM guard counts from the isolated, intentionally slower native proof."""
from pathlib import Path
import json,struct,subprocess,sys
elf,ram=sys.argv[1:]
raw=Path(ram).read_bytes()
want={'d9_verify_good','d9_verify_pcm','d9_verify_state','d9_verify_missed'}
symbols={}
for row in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
 c=row.split()
 if len(c)>=8 and c[7] in want and c[6]!='UND':
  a=int(c[1],16)-0x01c00000
  assert int(c[2])==4 and 0<=a<len(raw)-3
  symbols[c[7]]=struct.unpack_from('<I',raw,a)[0]
assert set(symbols)==want
print(json.dumps(symbols,sort_keys=True))
