from PIL import Image,ImageChops
from pathlib import Path
from hashlib import sha256
import sys
a,b=map(Path,sys.argv[1:]); paths=sorted(a.glob('*.png'))
assert len(paths)==len(list(b.glob('*.png')))==20
delta=lambda x,y: ImageChops.difference(Image.open(x).convert('RGB'),Image.open(y).convert('RGB'))
for f in paths:
 d=delta(f,b/f.name)
 r=d.getbbox()
 if f.name.startswith('fx-'):
  assert r and 3<=r[0]<r[2]<=237 and 112<=r[1]<r[3]<=156 and not d.crop((0,132,240,136)).getbbox(),(f.name,r)
  print(f.name,r,'summary only')
 else: assert not r,f.name
for theme in ('0','1'):
 for arm in (a,b):
  stem=f'fx-{theme}-tape-live'
  r=delta(arm/f'{stem}-a.png',arm/f'{stem}-b.png').crop((194,188,224,204))
  assert r.crop((0,0,14,16)).getbbox() and r.crop((16,0,30,16)).getbbox()
for owner in range(2):
 for arm in (a,b):
  stem=f'gen-{owner}-generate-play'
  d=delta(arm/f'{stem}-a.png',arm/f'{stem}-b.png')
  assert d.crop((3,20,237,143)).getbbox(), owner
pcm=(a/'fixture.pcm').read_bytes()
assert pcm==(b/'fixture.pcm').read_bytes()
print('PASS 12 FX summary-only, 8 GEN frames identical, both actual reels animate on admitted TAPE activity')
print('PASS actual host PCM bytes',len(pcm),'sha256',sha256(pcm).hexdigest())
