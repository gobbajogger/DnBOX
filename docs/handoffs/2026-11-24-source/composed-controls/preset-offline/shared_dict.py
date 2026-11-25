"""Offline exact preset packing candidates. No device format or disk corpus output."""
import sys, pathlib
sys.path.insert(0,str(pathlib.Path(__file__).parent/'source'))
from benchmark import Bits, Reader, huffman, table_for, write_table, read_table, select_table
from bench_corpus import load_j6, s1_load_zip, J6_HASHES
from install_bank import native_field, sid, SCHEMA
import collections, hashlib, struct, zlib, lzma, pickle, zstandard as zs, math, json, itertools

p16=struct.Struct('<H')
p32=struct.Struct('<I')
bh=struct.Struct('<4s8sHHI')
record=struct.Struct('<8sHBB')
def sect(n):return ((n+4095)//4096)
LZMA_HISTORY = int(__import__('os').environ.get('DNBOX_TRIAL_HISTORY', '65536'))
def codec(name, x, exact_size=None):
    if exact_size is None: 
        if name=='zlib':return zlib.compress(x,9)
        if name=='lzma':return lzma.compress(x,format=lzma.FORMAT_RAW,filters=[dict(id=lzma.FILTER_LZMA2,dict_size=LZMA_HISTORY,preset=9|lzma.PRESET_EXTREME)])
        if name=='zstd':return zs.ZstdCompressor(level=19).compress(x)
        if name=='raw':return x
    else:
        if name=='zlib':y=zlib.decompress(x)
        if name=='lzma':y=lzma.decompress(x,format=lzma.FORMAT_RAW,filters=[dict(id=lzma.FILTER_LZMA2,dict_size=LZMA_HISTORY)])
        if name=='zstd':y=zs.ZstdDecompressor().decompress(x,max_output_size=exact_size)
        if name=='raw':y=x
        assert len(y)==exact_size
        return y

def grouped(raw):
    out={}
    for p in raw:out.setdefault((p.engine,p.bank),[]).append(p)
    res={}
    for (e,b),ps in out.items():
        uid=ps[0].bank_uid or b
        counter=collections.Counter()
        a=[]
        for p in ps:
            counter[p.uid]+=1
            a.append((sid('preset',e,uid,p.uid,counter[p.uid]),p.pos,p.name.encode(),tuple((native_field(e,f),w)for f,w in p.fields)))
        res[e,b]=(sid('bank',e,uid),a)
    return res

def bitbytes(a,bits):
    b=Bits()
    for v in a:b.put(v,bits)
    return bytes(b.data)

def headers(rows,lm):
    return b''.join(record.pack(i,pos,len(n),lm[tuple(f for f,v in fs)])+n for i,pos,n,fs in rows)
def unheader(data,n):
    h=[];off=0
    for _ in range(n):
        i,p,k,l=record.unpack_from(data,off);off+=record.size
        h.append((i,p,data[off:off+k],l));off+=k
    assert off==len(data)
    return h

def huffman_model(banks,tuned):
    fw=collections.defaultdict(collections.Counter)
    for (e,b),(bankid,rows) in banks.items():
        for i,p,n,fs in rows:
            for f,w in fs:fw[e,f][w]+=1
    words=sorted({v for counts in fw.values() for v in counts}); wm={w:i for i,w in enumerate(words)}
    assert len(words)<=65535
    gb=(len(words)-1).bit_length()
    tables={}
    for k,counts in fw.items():
        r={wm[v]:n for v,n in counts.items()}
        if tuned:
            # Globally shared tables are paid once. Avoid many independent
            # dictionary copies; retain full rare-word escape precision.
            freq=[table_for(r,t) for t in (0,1,2,3,4,6,8)]
            def score(tab):
                buf=Bits();write_table(buf,tab,True);return buf.n+sum(n*(tab.codes[x][1] if x in tab.codes else tab.codes[-1][1]+gb)for x,n in r.items())
            tables[k]=min(freq,key=score)
        else:tables[k]=table_for(r,0)
    return words,wm,gb,tables

def shared(banks,name,tuned=True):
    words,wm,gb,tables=huffman_model(banks,tuned)
    layouts=list(dict.fromkeys((e,tuple(f for f,v in fs)) for (e,b),(bid,rows) in banks.items() for i,p,n,fs in rows))
    assert len(layouts)<256
    model=bytearray(struct.pack('<HHH',len(words),len(layouts),len(tables)))
    model+=b''.join(p32.pack(i)for i in words)
    for e,fs in layouts:model+=bytes([SCHEMA[e][0],len(fs),*fs])
    for (e,f),t in tables.items():
        q=Bits();write_table(q,t,True)
        model+=struct.pack('<BBH',SCHEMA[e][0],f,len(q.data))+q.data
    md=bytes(model);z=codec(name,md);q=codec(name,z,len(md));assert q==md
    nword,nlay,nt=struct.unpack_from('<HHH',q);off=6
    exact=struct.unpack_from('<'+'I'*nword,q,off);off+=nword*4
    inv={v[0]:k for k,v in SCHEMA.items()}
    layout=[]
    for _ in range(nlay):
        typ,n=q[off:off+2];off+=2;layout.append((inv[typ],tuple(q[off:off+n])));off+=n
    tt={}
    for _ in range(nt):
        typ,f,n=struct.unpack_from('<BBH',q,off);off+=4
        tt[inv[typ],f]=read_table(Reader(q[off:off+n]),True,nword);off+=n
    assert off==len(q)
    results=[]
    for (e,b),(bid,rows) in banks.items():
        lm={fs:i for i,(et,fs) in enumerate(layouts)if et==e}
        hs=headers(rows,lm);cs=codec(name,hs)
        pulse=Bits()
        for i,p,n,fs in rows:
            for f,w in fs:tables[e,f].emit(pulse,wm[w],gb)
        payload=bh.pack(b'XBD1',bid,len(rows),len(hs),len(cs))+cs+bytes(pulse.data)
        mg,bankid,m,hn,cn=bh.unpack_from(payload)
        head=unheader(codec(name,payload[bh.size:bh.size+cn],hn),m)
        rd=Reader(payload[bh.size+cn:]);dec=[]
        for i,p,n,l in head:
            label,fs=layout[l];assert label==e
            dec.append((i,p,n,tuple((f,exact[tt[e,f].read(rd,gb)])for f in fs)))
        assert mg==b'XBD1' and bankid==bid and dec==rows, b
        results.append((e,b,len(rows),len(payload),sect(len(payload))))
    return dict(codec=name,tuned=tuned,shared_bytes=len(z)+16,bank_bytes=sum(r[3]for r in results),bank_sectors=sum(r[4]for r in results),shared_sectors=sect(len(z)+16),banks=results)


# Use a frozen common model only for frequently authored values. All other
# binary32 words go into the current bank's escape stream as four byte planes.
def frequent(banks,name,minimum):
    fw=collections.defaultdict(collections.Counter)
    layouts=list(dict.fromkeys((e,tuple(f for f,v in fs)) for (e,b),(bid,rows) in banks.items() for i,p,n,fs in rows))
    assert len(layouts)<256
    for (e,b),(bid,rows) in banks.items():
        for i,p,n,fs in rows:
            for f,w in fs:fw[e,f][w]+=1
    words=sorted({v for counts in fw.values() for v,n in counts.items() if n>=minimum})
    wm={w:i for i,w in enumerate(words)}
    tables={}
    for k,counts in fw.items():
        fc={wm[v]:n for v,n in counts.items() if n>=minimum}
        fc[-1]=max(1,sum(n for v,n in counts.items() if n<minimum))
        tables[k]=canon(fc)
    model=bytearray(struct.pack('<HHH',len(words),len(layouts),len(tables)))
    model+=b''.join(p32.pack(i)for i in words)
    for e,fs in layouts:model+=bytes([SCHEMA[e][0],len(fs),*fs])
    for (e,f),t in tables.items():
        q=Bits();write_table(q,t,True)
        model+=struct.pack('<BBH',SCHEMA[e][0],f,len(q.data))+q.data
    md=bytes(model);z=codec(name,md);q=codec(name,z,len(md));assert q==md
    nword,nlay,nt=struct.unpack_from('<HHH',q);off=6
    exact=struct.unpack_from('<'+'I'*nword,q,off);off+=nword*4
    inv={v[0]:k for k,v in SCHEMA.items()}
    layout=[]
    for _ in range(nlay):
        typ,n=q[off:off+2];off+=2;layout.append((inv[typ],tuple(q[off:off+n])));off+=n
    tt={}
    for _ in range(nt):
        typ,f,n=struct.unpack_from('<BBH',q,off);off+=4
        tt[inv[typ],f]=read_table(Reader(q[off:off+n]),True,nword);off+=n
    assert off==len(q)
    results=[]
    hh=struct.Struct('<4s8sHHII')
    def encode_bank(e,bid,rows):
        lm={fs:i for i,(et,fs) in enumerate(layouts)if et==e}
        hs=headers(rows,lm)
        esc=[]
        pulse=Bits()
        for i,p,n,fs in rows:
            for f,w in fs:
                t=tables[e,f]; idx=wm.get(w,-1)
                if idx not in t.codes:idx=-1
                if idx==-1:esc.append(w)
                t.emit(pulse,idx)
        exc=b''.join(bytes((w>>shift)&255 for w in esc)for shift in range(0,32,8))
        allmeta=codec(name,hs+exc+bytes(pulse.data))
        payload=hh.pack(b'XBD2',bid,len(rows),len(hs),len(esc),len(hs+exc+pulse.data))+allmeta
        return payload+p32.pack(zlib.crc32(payload))
    def decode_bank(e,bid,payload):
        assert p32.unpack_from(payload,len(payload)-4)[0]==zlib.crc32(payload[:-4])
        mg,bankid,m,hn,en,un=hh.unpack_from(payload)
        decoded=codec(name,payload[hh.size:-4],un)
        head=unheader(decoded[:hn],m)
        ex_bytes=decoded[hn:hn+4*en]
        raises=iter(map(lambda x:sum(k<<s for k,s in zip(x,range(0,32,8))),zip(*(ex_bytes[i*en:(i+1)*en]for i in range(4)))))
        rd=Reader(decoded[hn+4*en:]);dec=[]
        for i,p,n,l in head:
            label,fs=layout[l];assert label==e
            dec.append((i,p,n,tuple((f,exact[ref] if(ref:=read_symbol(tt[e,f],rd))>=0 else next(raises)) for f in fs)))
        assert next(raises,None) is None
        assert mg==b'XBD2' and bankid==bid
        return dec

    extents={}
    for (e,b),(bid,rows) in banks.items():
        payload=encode_bank(e,bid,rows)
        assert decode_bank(e,bid,payload)==rows,b
        extents[e,b]=payload
        results.append((e,b,len(rows),len(payload),sect(len(payload))))

    # Remove one old record and replace a raw Float word with a previously
    # untrained finite value. Neither action rewrites another bank or model.
    target=max(banks,key=lambda k:len(banks[k][1]))
    e,b=target
    bid,rows=banks[target]; old=extents[target]
    nth=rows[0]; f,w=nth[3][0]
    unknown=next(v for v in range(0x3f111100,0x3f112100)if v not in wm)
    fresh=[(*nth[:3],((f,unknown),)+nth[3][1:])]+rows[2:]
    extents[target]=encode_bank(e,bid,fresh)
    assert extents[target]!=old and decode_bank(e,bid,extents[target])==fresh
    del extents[target]
    for k,payload in extents.items():assert decode_bank(k[0],banks[k][0],payload)==banks[k][1]
    return dict(codec=name,minimum=minimum,checked_replace_and_remove=True,shared_bytes=len(z)+16,bank_bytes=sum(r[3]for r in results),bank_sectors=sum(r[4]for r in results),shared_sectors=sect(len(z)+16),banks=results)

def canon(f):
    from benchmark import Table
    lens=huffman(f);syms=sorted(f,key=lambda x:(lens[x],x));code=0;prev=0;codes={}
    for x in syms:
        n=lens[x];code<<=n-prev;codes[x]=code,n;code+=1;prev=n
    return Table(syms,[lens[x] for x in syms],codes,True)

def read_symbol(t,r):
    if len(t.syms)==1 and t.lengths[0]==0:return t.syms[0]
    v=first=pos=0
    for n in range(1,16):
        v=(v<<1)|r.get(1);count=t.lengths.count(n)
        if first<=v<first+count:return t.syms[pos+v-first]
        pos+=count;first=(first+count)<<1
    raise ValueError('invalid shared Huffman code')

if __name__=='__main__':
    j,m=load_j6(sys.argv[1]);assert sorted(m['member_sha256'])==sorted(J6_HASHES)
    s,sm=s1_load_zip(sys.argv[2]);assert len(j)==640 and len(s)==906
    banks=grouped(j+s);assert sum(len(p.fields)for p in j+s)==221013
    if len(sys.argv)>3 and sys.argv[3]=='frequent':
        for n in (2,4,8,16,32):
            for name in ('lzma','zstd'):
                print(json.dumps(frequent(banks,name,n)),flush=True)
    else:
        for name in ['zstd','lzma','zlib']:
            print(json.dumps(shared(banks,name)),flush=True)
