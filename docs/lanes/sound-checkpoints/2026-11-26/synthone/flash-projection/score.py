import re,subprocess,json,sys
from pathlib import Path
def run(flag,p): return subprocess.check_output(['readelf',flag,str(p)],text=True)
def bytes_for(p):
    out={}
    for line in run('-SW',p).splitlines():
        m=re.match(r'^\s*\[\s*\d+\]\s+(\.\S+)\s+(\S+)\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\S+\s+(\w+)',line)
        if m and m[2]=='PROGBITS' and 'A' in m[4]:
            k='text' if 'X' in m[4] else 'initialized_data' if 'W' in m[4] else 'read_only'
            out[k]=out.get(k,0)+int(m[3],16)
    return dict(allocated_flash_sections=out,bytes=sum(out.values()),unresolved=[l.split()[-1] for l in run('-sW',p).splitlines() if re.search(r'\bUND\s+\S+',l)])
d=Path(sys.argv[1])
z={n:bytes_for(d/(n+'.o')) for n in ('baseline-engine','matched-engine','baseline-engine-with-runtime','matched-engine-with-runtime','fx-subgraph')}
z['deltas']={x:z['matched-engine'+x]['bytes']-z['baseline-engine'+x]['bytes'] for x in ('','-with-runtime')}
print(json.dumps(z,indent=2))
