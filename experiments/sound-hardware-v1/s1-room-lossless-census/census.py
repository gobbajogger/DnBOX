"""Offline capacity census. Not a mutable-ring or codec fit recommendation."""
import hashlib,json,lzma,struct,sys,zlib
from pathlib import Path
payload=Path(sys.argv[1]).read_bytes()
stride=98760
assert len(payload)%stride==0
def xor_each(b):
    words=struct.unpack("<%dI"%(len(b)//4),b)
    return struct.pack("<%dI"%len(words),*(a^p for a,p in zip(words,(0,*words[:-1]))))
def shuffle(b): return b"".join(b[i::4] for i in range(4))
def sizes(b):
    values=[b,shuffle(b),xor_each(b),shuffle(xor_each(b))]
    return min(len(zlib.compress(v,9)) for v in values),min(len(lzma.compress(v,preset=6)) for v in values)
rows=[]
for t,i in enumerate(range(0,len(payload),stride),1):
    b=payload[i:i+stride]
    d,x=sizes(b)
    for page in (256,1024):
        offset_table=4*((len(b)+page-1)//page+1)
        # optimistic best of all representations; not charging per-block mode bytes.
        enc=sum(min(len(b[j:j+page]),sizes(b[j:j+page])[0]) for j in range(0,len(b),page))
        if page==256: fixed256=enc+offset_table
        else: fixed1024=enc+offset_table
    rows.append([t*4096,d,x,fixed256,fixed1024])
assert rows
warm=[r for r in rows if 32768<=r[0]<=180224]
print(json.dumps(dict(source_history_bytes=stride,source_history_sha256=hashlib.sha256(payload).hexdigest(),
    columns=["frame","whole_zlib","whole_xz","page256_zlib_including_index","page1024_zlib_including_index"],
    sampled=len(rows),warm_sustained_extents=[(min(r[i] for r in warm),max(r[i] for r in warm)) for i in range(1,5)],
    rows=rows),indent=2))
