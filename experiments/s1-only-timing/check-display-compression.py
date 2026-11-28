from pathlib import Path
from PIL import Image
import struct,itertools,zlib
root=Path('/tmp/dnbox-ram')
rows=[]
for d in ('display/compacted','display64/baseline-out'):
    for img in (root/d).glob('*.png'):
        im=Image.open(img).convert('RGB')
        assert im.size == (240,240), im.size
        for y,h,name in ((0,20,'header'),(20,69,'main0'),(89,69,'main1'),(158,82,'footer')):
            px=[(r>>3)*2048+(g>>2)*32+(b>>3) for r,g,b in im.crop((0,y,240,y+h)).getdata()]
            src=struct.pack('<'+'H'*len(px),*px)
            rle=bytearray()
            for color,grp in itertools.groupby(px):
                run=sum(1 for _ in grp)
                while run:
                    n=min(run,256)
                    rle.extend(struct.pack('<HB',color,n-1))
                    run-=n
            out=[]
            for c,n in struct.iter_unpack('<HB',rle):
                out.extend([c]*(n+1))
            assert out==px
            compressed=zlib.compress(src,1)
            assert zlib.decompress(compressed)==src
            rows.append((name,str(img.relative_to(root)),len(set(px)),len(rle),len(compressed),len(src)))
assert rows
print('frame count:',len(rows)//4)
for name in ('header','main0','main1','footer'):
    p=[x for x in rows if x[0]==name]
    print(name, 'cases',len(p),'bytes',p[0][5], 'max colors',max(x[2] for x in p),'#>256 colors',sum(x[2]>256 for x in p),'max rle',max(x[3] for x in p),'max zlib',max(x[4] for x in p),'rle worst file',max(p,key=lambda x:x[3])[1])
