import struct,sys,json,hashlib
from pathlib import Path
Q=Path('fixed-traffic-audit')
ram=Path('/tmp/dnbox-loaded-replay-pro-0.GcdgKnst/ram.bin').read_bytes()
assert hashlib.sha256(ram).hexdigest()=='830c4a89349c9c872eb67a91059afbf72cfcefdbdb21741ca7ebb9d8cab99120'
c=json.loads(Path('strict-warmfold-pro-c1/result.json').read_text())
assert hashlib.sha256(Path(sys.argv[2]).read_bytes()).hexdigest()==c['elf']
A,a,b,n,B,p,S,T,R,D=struct.unpack('<10I',Path(sys.argv[1]).read_bytes())
syms={name:(int(addr,16)-0x01c00000,int(sz,16)) for addr,sz,typ,name in map(str.split,(Q/'symbols.txt').read_text().splitlines())}
u,v=syms['d909'];j,k=syms['d808'];assert (v,k)==(A,B),(A,B,v,k)
f=lambda i,n:struct.unpack_from('<'+str(n)+'f',ram,i)
print(json.dumps(dict(elf=c['elf'],ram=c['ram_sha'],d909_rev=f(u+a,n),d909_delay=f(u+b,n),
    d808_rev=[f(j+p+4*(i*T+R),1)[0] for i in range(S)],
    d808_delay=[f(j+p+4*(i*T+D),1)[0] for i in range(S)],
    scope='Final RAM only; not a read at missed half 484, not evidence these stayed fixed during scene.'),indent=2))
