from pathlib import Path
from PIL import Image,ImageChops
import sys,re
a,b=map(Path,sys.argv[1:])
same=focus=0
imgs=sorted(a.glob('*.png'))
assert len(imgs)==len(list(b.glob('*.png')))==98
for f in imgs:
    prev=Image.open(f).convert('RGB'); cur=Image.open(b/f.name).convert('RGB')
    d=ImageChops.difference(prev,cur)
    r=d.getbbox()
    m=re.fullmatch(r'[AB]-(SJ6-LFO-RATE|S1-EFFECT|S1-OUTPUT)-(\d+)-(low|high)\.png',f.name)
    if m:
        focus+=1
        panel,n,_=m.groups(); k=int(n)-1; w=118 if panel=='S1-OUTPUT' else 59
        x0=12+k*w
        assert r is not None and r[0]==x0 and r[2]==x0+w-17 and r[1] in (70,120,170,220) and r[3]==r[1]+2, (f.name,r)
        rect=cur.crop(r)
        assert len(rect.getcolors())==1 and d.crop((0,0,240,20)).getbbox() is None, f.name
        print(f.name,r)
    else:
        same+=1
        assert r is None, (f.name,r)
assert (focus,same)==(40,58),(focus,same)
print('PASS 40 exact focused-cell raster pairs, 58 completely unchanged expired/default/navigation pairs.')
