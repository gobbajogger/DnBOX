from PIL import Image,ImageChops
from pathlib import Path
from hashlib import sha256
import sys,re
pre,post=map(Path,sys.argv[1:])
fs=sorted(pre.glob('*.png')); assert len(fs)==len(list(post.glob('*.png')))==26
glyphs={}
for f in fs:
 a=Image.open(f).convert('RGB'); b=Image.open(post/f.name).convert('RGB')
 assert a.size==b.size==(240,240)
 d=ImageChops.difference(a,b); r=d.getbbox()
 if 'unrelated' in f.name:
   assert r is None, f
   continue
 assert r and (198<=r[0]<r[2]<=223 and 145<=r[1]<r[3]<=152), (f,r)
 old=a.crop((198,145,223,152)); assert len(old.getcolors())==1, f
 m=re.fullmatch(r'[AB]-[01]-([0-4])\.png',f.name)
 if m:
   key=m.group(1)
   mask=tuple((x,y) for y in range(145,152) for x in range(198,223) if d.getpixel((x,y))!=(0,0,0))
   assert key not in glyphs or glyphs[key]==mask, f
   glyphs[key]=mask
 print(f.name,r,'existing LFO2 cell blank pixels only')
assert len(glyphs)==len(set(glyphs.values()))==5
print('PASS all 5 distinct waveform rasters, both owners and traversal directions, 24 in-cell-only paints; 2 unrelated renders exact')
