import concurrent.futures, hashlib, tempfile, subprocess
from pathlib import Path
r=Path.cwd()
d=r/'s1-focus-keycache/host-compare';pack=r/'fw/build/smk37-elite/fdata.img'
b=pack.read_bytes();assert b[:4]==b'S1PK'
n=int.from_bytes(b[6:8],'little');assert n==694
print(f'pack={hashlib.sha256(b).hexdigest()} programs={n}',flush=True)
def check(i):
    with tempfile.TemporaryDirectory(prefix='dnbox-key-cache-cmp-') as name:
        p=Path(name);buffers=[]
        for exe in ('Oz','O2'):
            o=p/exe
            subprocess.run([d/exe,pack,str(i),o,'wet'],check=True,stdout=subprocess.DEVNULL)
            buffers.append(o.read_bytes())
        assert len(buffers[0])>0 and buffers[0]==buffers[1],i
        return hashlib.sha256(buffers[0]).digest()
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as ex:
    h=hashlib.sha256()
    for i,x in enumerate(ex.map(check,range(n)),1):
        h.update(x)
        if i%100==0:print(i,flush=True)
print(f'{n}/{n} bit-exact PCM; aggregate={h.hexdigest()}',flush=True)
