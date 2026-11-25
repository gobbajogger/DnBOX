"""Cross-column adaptive entropy witness. No stored probability vectors."""
import sys, json, struct, collections, zlib, numpy as np
from predict_exact import *
from shared_dict import Bits,Reader
MASK=(1<<32)-1; Q=(1<<30); H=2*Q
class Freq:
    def __init__(self,n):
        self.n=n;self.bit=[0]*(n+1);self.total=0
    def add(self,i,v=1):
        self.total+=v;i+=1
        while i<=self.n:self.bit[i]+=v;i+=i&-i
    def prefix(self,i):
        s=0
        while i:s+=self.bit[i];i&=i-1
        return s
    def span(self,i):return self.prefix(i),self.prefix(i+1)
    def lookup(self,p):
        i=0;a=1<<(self.n.bit_length()-1)
        while a:
            k=i+a
            if k<=self.n and self.bit[k]<=p:i=k;p-=self.bit[k]
            a>>=1
        return i

def encode(values,flag):
    b=Bits();lo=0;hi=MASK;pending=0
    m=Freq(len(values)+1);m.add(0)
    d={};word_order=[]
    for value in values:
        sym=d.get(value,0)
        low,high=m.span(sym);dist=hi-lo+1
        hi=lo+dist*high//m.total-1
        lo+=dist*low//m.total
        while 1:
            if hi<H or lo>=H:
                k=int(lo>=H);b.put(k,1)
                if pending:b.put((1<<pending)-1 if not k else 0,pending);pending=0
                if k:lo-=H;hi-=H
            elif lo>=Q and hi<3*Q:pending+=1;lo-=Q;hi-=Q
            else:break
            hi=2*hi+1;lo*=2
        if not sym:
            word_order.append(value);sym=len(word_order);d[value]=sym
            # PPM escape weight as in different empirically tuned estimators.
            if flag==1:m.add(0)
        m.add(sym)
    k=int(lo>=Q);b.put(k,1);pending+=1
    if pending:b.put((1<<pending)-1 if not k else 0,pending)
    return word_order,bytes(b.data)

def decode(data,words,length,flag):
    r=Reader(data+bytes(4));lo=0;hi=MASK;read=r.get(32)
    m=Freq(length+1);m.add(0)
    have=[None]
    for _ in range(length):
        dist=hi-lo+1;p=((read-lo+1)*m.total-1)//dist
        sym=m.lookup(p);low,high=m.span(sym)
        hi=lo+dist*high//m.total-1
        lo+=dist*low//m.total
        while 1:
            if hi<H or lo>=H:
                if lo>=H:lo-=H;hi-=H;read-=H
            elif lo>=Q and hi<3*Q:lo-=Q;hi-=Q;read-=Q
            else:break
            hi=2*hi+1;lo*=2;read=2*read+r.get(1)
        if not sym:
            have.append(next(words));sym=len(have)-1
            if flag==1:m.add(0)
        m.add(sym)
        yield have[sym]

def run(banks,method,comp):
    order,layouts,original=flatten(banks);rows=distance_order(original,layouts,method)
    fields=collections.defaultdict(list)
    for bank,id,pos,n,shape,fs in rows:
        et=SCHEMA[order[bank][0]][0]
        for f,w in fs:fields[et,f].append(w)
    allwords=U32(sorted({w for vs in fields.values()for w in vs}))
    wm={int(w):i for i,w in enumerate(allwords)}
    gs=sh(np.diff(allwords,prepend=U32([0])));body=bytearray(gs)
    for (e,f),values in sorted(fields.items()):
        # Escape sequences are encoded as ordered first-use references to
        # global exact binary32 words; the arithmetic model learns from the
        # emitted sequence and stores no histogram/lengths.
        choices=[]
        for flag in (0,1):
            w,t=encode(values,flag); refs=U16([wm[v]for v in w])
            shrefs=sh(np.diff(refs,prepend=U16([0])))
            choices.append((len(codec(comp,shrefs+t)),flag,w,t,shrefs))
        _,flag,w,t,shrefs=min(choices,key=lambda x:x[0])
        body+=struct.pack('<BBHHHB',e,f,len(values),len(w),len(t),flag)+shrefs+t
    meta=metadata(rows,order,layouts,banks)
    m=codec(comp,meta);v=codec(comp,bytes(body))
    hdr=struct.Struct('<4sHHHIIII')
    magic={'zstd':b'PADZ','lzma':b'PADL'}[comp]
    raw=hdr.pack(magic,len(rows),len(fields),len(allwords),len(meta),len(m),len(body),len(v))+m+v
    raw+=struct.pack('<I',zlib.crc32(raw))
    mg,n,nf,ng,ml,ms,vl,vs=hdr.unpack_from(raw)
    assert mg==magic and len(raw)==hdr.size+ms+vs+4 and struct.unpack_from('<I',raw,len(raw)-4)[0]==zlib.crc32(raw[:-4])
    bb,ll,rr=unmeta(codec(comp,raw[hdr.size:hdr.size+ms],ml),n)
    storage=codec(comp,raw[hdr.size+ms:-4],vl)
    gg=np.add.accumulate(ush(storage[:ng*4],4),dtype='<u4');off=ng*4
    dcols={}
    for n in range(nf):
        e,f,nv,nw,nt,flag=struct.unpack_from('<BBHHHB',storage,off);off+=9
        ww=np.add.accumulate(ush(storage[off:off+nw*2],2),dtype='<u2');off+=nw*2
        used=iter(int(gg[i])for i in ww)
        arr=list(decode(storage[off:off+nt],used,nv,flag));off+=nt
        assert next(used,None)is None and arr==fields[e,f]
        dcols[e,f]=iter(arr)
    assert off==len(storage)
    check=collections.defaultdict(list)
    for bank,id,pos,n,s in rr:
        e,b,bid=bb[bank]; el,fs=ll[s];assert e==el and bid==banks[e,b][0]
        out=tuple((f,next(dcols[SCHEMA[e][0],f]))for f in fs)
        check[e,b].append((id,pos,n,out))
    rrkey=lambda p:(p[1],p[0])
    assert all(sorted(val,key=rrkey)==sorted(check[k],key=rrkey)for k,(bid,val)in banks.items())
    assert all(next(q,None)is None for q in dcols.values())
    return dict(order=method,total=len(raw),meta=len(m),value=len(v),header=hdr.size+4,tight=sect(len(raw))*4096,
                codec=comp,globals=len(allwords),presets=len(rows),words=sum(map(len,fields.values())),exact=True)
if __name__=='__main__':
    j,m=load_j6(sys.argv[1]);assert sorted(m['member_sha256'])==sorted(J6_HASHES)
    s,sm=s1_load_zip(sys.argv[2]);assert len(j)==640 and len(s)==906
    for method in ('source','absolute'):
        for comp in ('zstd','lzma'):
            print(json.dumps(run(grouped(j+s),method,comp)),flush=True)
