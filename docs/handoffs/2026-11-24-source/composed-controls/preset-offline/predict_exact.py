"""Offline predictor measurements; serializes private bytes in process memory only."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from shared_dict import grouped, load_j6, s1_load_zip, J6_HASHES, SCHEMA, codec, sect
import collections, itertools, struct, json, zlib, lzma, numpy as np
CHDR=struct.Struct('<BBHHI')   # engine, numeric field, count, slots, payload bytes
HEAD=struct.Struct('<4sBHHIIII') # version, codec, rows, columns, metadata plain/stored, values plain/stored
U16=lambda a:np.asarray(a,dtype='<u2')
U32=lambda a:np.asarray(a,dtype='<u4')

def sh(a):
    return a.view('u1').reshape(-1,a.dtype.itemsize).T.copy().tobytes()
def ush(b,size):
    if not b:return np.empty(0,dtype='<u%d'%size)
    return np.frombuffer(b,dtype='u1').reshape(size,-1).T.copy().view('<u%d'%size)[:,0]
def h(x):return struct.pack('<H',x)
def flatten(banks):
    order=list(banks)
    # All bank and preset stable IDs retained inline. Their names specific to
    # rights-restricted local source and are not written to evidence output.
    layouts=[];rows=[]
    for no,((e,b),(bid,ps))in enumerate(banks.items()):
        for id,pos,n,fs in ps:
            fields=tuple(f for f,_ in fs)
            x=(e,fields)
            if x not in layouts:layouts.append(x)
            rows.append((no,id,pos,n,layouts.index(x),fs))
    return order,layouts,rows

def distance_order(rows,layouts,method):
    if method=='source':return rows
    result=[]
    for eng in ['SuperJ6','Synth One']:
        r=[x for x in rows if layouts[x[4]][0]==eng]
        # 64-bit sentinel distinct from any finite binary32 bit pattern.
        mat=np.full((len(r),SCHEMA[eng][1]),0x100000000,dtype='u8')
        for i,(_,id,pos,n,j,fs) in enumerate(r):
            for f,w in fs:mat[i,f]=w
        n=len(r); dist=np.zeros((n,n),dtype='f4')
        for f in range(mat.shape[1]):
            a=mat[:,f]
            freq=collections.Counter(map(int,a))
            if len(freq)<=1:continue
            changed=(a[:,None]!=a[None,:])
            if method=='rare':
                w=np.asarray([min(4,1+.45*np.log(n/freq[int(v)])) for v in a],dtype='f4')
                dist+=changed*(w[:,None]+w[None,:])*.5
            else:dist+=changed
        remaining=set(range(n)); seq=[]; cur=int(np.argmin(dist.mean(axis=1)))
        while remaining:
            seq.append(cur);remaining.remove(cur)
            if remaining:cur=min(remaining,key=lambda t:(dist[cur,t],t))
        result.extend(r[i]for i in seq)
    return result

def model(values):
    # Four alternatives: shuffled exact words, predecessor XOR/subtraction,
    # or sorted Float32 palette + predecessor difference of codes. A codec
    # byte is part of every stored column, so adaptive selection is honest.
    n=len(values); pp=np.r_[U32([0]),values[:-1]]
    yield 0,sh(values)
    yield 1,sh(values^pp)
    yield 2,sh(values-pp)
    g,i=np.unique(values,return_inverse=True); i=U16(i)
    dg=np.diff(g,prepend=U32([0]))
    for delta,a in ((0,i),(1,np.diff(i,prepend=U16([0])))):
        yield 3+delta,h(len(g))+sh(dg)+sh(a)
    # Within each exponent group exact words often lie on an integer grid
    # (cutoff, level and semitone knobs). Factor the GCD of sorted ULP gaps;
    # division/multiplication is exact uint32 with finite kept as bit words.
    for delta,a in ((0,i),(1,np.diff(i,prepend=U16([0])))):
        if len(g)<=1:continue
        gaps=np.diff(g).astype('<u4'); d=np.gcd.reduce(gaps)
        if d<=1:continue
        yield 5+delta,h(len(g))+struct.pack('<II',int(g[0]),int(d))+sh(gaps//d)+sh(a)
    # Similar musically adjacent patches often inherit most columns;
    # bitmask unchanged values and store only exact replacements.
    for delta in [0,1]:
        mask=values!=pp; pick=values[mask] if delta==0 else (values^pp)[mask]
        yield 7+delta,np.packbits(mask).tobytes()+sh(pick)

def demodel(i,x,n):
    if i<3:
        y=ush(x,4)
        if i==1:y=np.bitwise_xor.accumulate(y)
        elif i==2:y=np.add.accumulate(y,dtype='<u4')
        assert len(y)==n;return y
    if i in (3,4,5,6):
        k=struct.unpack_from('<H',x)[0]
        if i<=4:
            g=np.add.accumulate(ush(x[2:2+k*4],4),dtype='<u4'); a=ush(x[2+k*4:],2)
        else:
            g0,d=struct.unpack_from('<II',x,2)
            w=ush(x[10:10+(k-1)*4],4)*np.uint32(d)
            g=np.add.accumulate(np.r_[U32([g0]),w],dtype='<u4');a=ush(x[10+(k-1)*4:],2)
        if i%2==0:a=np.add.accumulate(a,dtype='<u2')
        assert len(g)==k and len(a)==n
        return g[a]
    m=(n+7)//8
    mask=np.unpackbits(np.frombuffer(x[:m],dtype='u1'))[:n]
    v=ush(x[m:],4); out=[]; p=0;j=0
    for s in mask:
        if s:
            p=int(v[j])^(p if i==8 else 0);j+=1
        out.append(p)
    assert j==len(v)
    return U32(out)

def metadata(rows,order,layouts,banks):
    p=bytearray(h(len(order))+h(len(layouts)))
    for e,b in order:
        bid,_=banks[e,b]; n=b.encode(); assert len(n)<=255
        p+=bytes([SCHEMA[e][0],len(n)])+bid+n
    for e,fs in layouts:p+=bytes([SCHEMA[e][0],len(fs),*fs])
    for bank,id,pos,n,shape,fs in rows:
        p+=struct.pack('<BBH8sB',bank,shape,pos,id,len(n))+n
    return bytes(p)

def unmeta(raw,cnt):
    nb,nl=struct.unpack_from('<HH',raw);o=4
    eids={v[0]:k for k,v in SCHEMA.items()}
    banks=[]
    for _ in range(nb):
        e,n=raw[o:o+2];banks.append((eids[e],raw[o+10:o+10+n].decode(),raw[o+2:o+10]));o+=10+n
    layouts=[]
    for _ in range(nl):
        e,n=raw[o:o+2];o+=2;layouts.append((eids[e],tuple(raw[o:o+n])));o+=n
    rows=[]
    for _ in range(cnt):
        bank,shape,pos,id,n=struct.unpack_from('<BBH8sB',raw,o);o+=13
        rows.append((bank,id,pos,raw[o:o+n],shape));o+=n
    assert o==len(raw)
    return banks,layouts,rows

def crossmodels(vals,options,comp):
    # Decode dependencies always point to an earlier equal-support column.
    # No row indices need shipping: support order reconstructed from exact
    # row layouts. Compare correlation before entropy candidate trials.
    n=len(vals)
    ranked=[]
    for f,v in options:
        d=(v.astype('i8')-vals.astype('i8'))
        tight=np.count_nonzero(abs(d)<256)
        distinct=len(set(zip(v,vals)))
        ranked.append((int(tight)-distinct//4, f, v))
    for _,f,v in sorted(ranked,key=lambda x:x[0],reverse=True)[:3]:
        # Numerically similar cutoff/gain/arp lanes: first-order exact ULP
        # and sparse checked overlay. Modulo arithmetic reverses exactly.
        raw=vals-v
        yield 9,bytes([f])+sh(raw)
        mask=raw!=0
        yield 10,bytes([f])+np.packbits(mask).tobytes()+sh(raw[mask])
        # Learn a discrete contextual map only when repeatedly useful.
        # Storage contains predictor keys and exact baseline for each key.
        pair=collections.defaultdict(collections.Counter)
        for b,a in zip(v,vals):pair[int(b)][int(a)]+=1
        pred={k:c.most_common(1)[0][0] for k,c in pair.items() if c.most_common(1)[0][1]>=4}
        if not pred or len(pred)>96:continue
        base=U32([pred.get(int(k),0)for k in v]);res=vals-base
        mask=res!=0
        yield 11,bytes([f,len(pred)])+b''.join(struct.pack('<II',k,w)for k,w in sorted(pred.items()))+np.packbits(mask).tobytes()+sh(res[mask])

def uncross(k,x,n,prior):
    base=prior[x[0]]
    assert len(base)==n
    if k==9:return base+ush(x[1:],4)
    if k==11:
        ct=x[1]
        m=dict(struct.iter_unpack('<II',x[2:2+8*ct]))
        base=U32([m.get(int(v),0)for v in base]);x=x[1+8*ct:]
    mask=np.unpackbits(np.frombuffer(x[1:1+(n+7)//8],dtype='u1'))[:n].astype('bool')
    out=base.copy();out[mask]+=ush(x[1+(n+7)//8:],4)
    return out

def evaluate(banks,method,comp,cross=False):
    order,layouts,original=flatten(banks); rows=distance_order(original,layouts,method)
    parts=collections.defaultdict(list); supports=collections.defaultdict(list)
    # Use field occurrence index for rare duplicate ordered fields in an
    # authentic layout: decoder consumes in precisely that layout order.
    for row,(bank,id,pos,n,shape,fs) in enumerate(rows):
        et=SCHEMA[order[bank][0]][0]
        for f,w in fs:
            parts[et,f].append(w); supports[et,f].append(row)
    m=metadata(rows,order,layouts,banks);menc=codec(comp,m)
    stamps=collections.Counter();stream=bytearray(); local={}; linked=collections.defaultdict(list)
    for (et,f),vals in sorted(parts.items()):
        a=U32(vals)
        # Column-local codec scoring chooses a single lossless predictor,
        # then the concatenated descriptor+payload stream shares final codec
        # across all columns/engines (does not store temporary trial sizes).
        support=(et,tuple(supports[et,f]))
        options=linked[support]
        choices=itertools.chain(model(a),crossmodels(a,options,comp)) if cross else model(a)
        k,raw=min(choices,key=lambda i:len(codec(comp,i[1])))
        assert np.array_equal(a,demodel(k,raw,len(a)) if k<9 else uncross(k,raw,len(a),local[et])), (et,f,k)
        linked[support].append((f,a))
        local.setdefault(et,{})[f]=a
        stamps[k]+=1
        stream+=CHDR.pack(et,f,len(a),k,len(raw))+raw
    venc=codec(comp,bytes(stream))
    h=HEAD.pack(b'PXF1',{'zlib':0,'zstd':1,'lzma':2}[comp],len(rows),len(parts),len(m),len(menc),len(stream),len(venc))
    blob=h+menc+venc
    blob+=struct.pack('<I',zlib.crc32(blob))
    assert zlib.crc32(blob[:-4])==struct.unpack_from('<I',blob,len(blob)-4)[0]
    magic,c,rc,nc,mn,ms,vn,vs=HEAD.unpack_from(blob)
    assert magic==b'PXF1' and rn(c)==comp
    dm=codec(comp,blob[HEAD.size:HEAD.size+ms],mn)
    dv=codec(comp,blob[HEAD.size+ms:-4],vn)
    bb,ll,rr=unmeta(dm,rc); dcols={};o=0; prior=collections.defaultdict(dict)
    for _ in range(nc):
        e,f,count,k,n=CHDR.unpack_from(dv,o);o+=CHDR.size
        raw=dv[o:o+n]
        col=demodel(k,raw,count) if k<9 else uncross(k,raw,count,prior[e])
        prior[e][f]=col
        dcols[e,f]=iter(map(int,col));o+=n
    assert o==len(dv)
    check=collections.defaultdict(list)
    for bank,id,pos,n,s in rr:
        e,b,bid=bb[bank];el,fs=ll[s];assert e==el
        assert bid==banks[e,b][0]
        out=[(f,next(dcols[SCHEMA[e][0],f]))for f in fs]
        check[e,b].append((id,pos,n,tuple(out)))
    rrkey=lambda p:(p[1],p[0])
    assert all(sorted(val,key=rrkey)==sorted(check[k],key=rrkey)for k,(bid,val)in banks.items())
    assert all(next(q,None)is None for q in dcols.values())
    return dict(order=method,codec=comp,cross=cross,metadata=len(menc),values=len(venc),
                crc_header=len(h)+4,total=len(blob),ideal_sector_bytes=sect(len(blob))*4096,
                column_count=len(parts),predictor_count=stamps,words=sum(len(fs) for _,_,_,_,_,fs in rows),
                presets=len(rows),exact=True)

def rn(c):return ['zlib','zstd','lzma'][c]

if __name__=='__main__':
    j,m=load_j6(sys.argv[1]);assert sorted(m['member_sha256'])==sorted(J6_HASHES)
    s,sm=s1_load_zip(sys.argv[2]);assert len(j)==640 and len(s)==906
    banks=grouped(j+s)
    if len(sys.argv)>3 and sys.argv[3]=='cross':
        print(json.dumps(evaluate(banks,'absolute','lzma',True)),flush=True)
    else:
        for method in ('source','absolute','rare'):
            for comp in ('zstd','lzma'):
                print(json.dumps(evaluate(banks,method,comp)),flush=True)
