#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline Chicago filter table experiments: exact IEEE words, bounded random access."""
import re
import struct
import sys
from pathlib import Path

from fractions import Fraction
from functools import lru_cache

U32=(1<<32)-1
Q=20
FAST=True
COUNT=16385

def input_words(text,name):
    def array(suffix):
        m=re.search('original_chicago_packed_'+name+'_'+suffix+r'\[\d+\]=\{(.*?)\};',text,re.S)
        return list(map(int,m[1].split(','))) if not m[1].rstrip().endswith(',') else [int(x) for x in m[1].split(',') if x.strip()]
    data=bytes(array('data'));offsets=array('offset')
    v=[]
    for k,p in enumerate(offsets):
        n=min(32,COUNT-32*k)
        x,d,w=struct.unpack_from('<IiB',data,p)
        bits=int.from_bytes(data[p+9:offsets[k+1] if k+1<len(offsets) else len(data)],'little')
        for j in range(n):
            if j==1:
                x+=d
            elif j>=2:
                z=(bits >> ((j-2)*w))&((1<<w)-1)
                d+=(z>>1)^-(z&1)
                x+=d
            v.append(x&U32)
    assert len(v)==COUNT
    return data,offsets,v

@lru_cache(maxsize=None)
def gram_inverse(n, degree):
    # Exact rational Gaussian elimination, at most four rows. Fit on integer
    # sample positions then scale the coefficients to the decoder's i/128.
    m=degree+1
    moments=[sum(i**k for i in range(n)) for k in range(2*m-1)]
    a=[[Fraction(moments[i+j]) for j in range(m)]+[Fraction(i==j) for j in range(m)] for i in range(m)]
    for k in range(m):
        pivot=a[k][k]
        a[k]=[v/pivot for v in a[k]]
        for j in range(m):
            if j!=k:
                scale=a[j][k]
                a[j]=[x-scale*y for x,y in zip(a[j],a[k])]
    return [row[m:] for row in a]

def encoded_segment(words,nmax):
    n=len(words)
    shift=nmax.bit_length()-1
    assert nmax==1<<shift
    base=words[0]
    best=None
    rhs=[sum(i**j*(w-base) for i,w in enumerate(words)) for j in range(min(n,4))]
    for degree in range(min(n,4)):
        co=[sum(v*x for v,x in zip(row,rhs))*nmax**i for i,row in enumerate(gram_inverse(n,degree))]
        bound=sum(map(abs,co))
        q=next((q for q in range(18,-1,-1) if (bound*(1<<q)+degree+1)*nmax<=(1<<31)-1),0)
        poly=tuple(round(c*(1<<q)) for c in co)
        vals=[]
        admissible=True
        for i,word in enumerate(words):
            v=poly[-1]
            for a in reversed(poly[:-1]):
                lim=(1<<31) if FAST else (1<<63)
                if not -lim<=v*i<lim:
                    admissible=False
                    break
                v=a+((v*i)>>shift)
                if FAST and not -(1<<31)<=v+(1<<(max(q,1)-1))<(1<<31):
                    admissible=False;break
            if not admissible:break
            vals.append(word-(base+(((v+(1<<(q-1)))>>q) if q else v)))
        if not admissible:continue
        lo=min(vals); hi=max(vals)
        width=(hi-lo).bit_length()
        if width>31 or not -(1<<31)<=lo<(1<<31):continue
        bits=0
        for i,d in enumerate(vals):bits|=(d-lo)<<(i*width)
        # i32 coefficients when possible; exact escape i64 otherwise. Constant
        # term is folded into the base except for the fractional component.
        c=poly[0]>>q
        base2=base+c
        poly=(poly[0]-(c<<q),*poly[1:])
        if not 0<=base2<=U32:continue
        coeff=bytes([q]) if FAST else b''
        for p in poly:
            z=(p<<1) if p>=0 else ((-p<<1)-1)
            while z>=128:
                coeff+=bytes([(z&127)|128]);z>>=7
            coeff+=bytes([z])
        record=struct.pack('<BIi',(degree<<5)|width,base2,lo)+coeff+bits.to_bytes((n*width+7)//8,'little')
        if best is None or len(record)<len(best):
            best=record
    assert best is not None
    return best

def one(data,start,i,seg):
    h,base,lo=struct.unpack_from('<BIi',data,start)
    degree=h>>5
    w=h&31
    p=start+9
    q=data[p] if FAST else Q
    p+=int(FAST)
    poly=[]
    for _ in range(degree+1):
        k=z=0
        while True:
            b=data[p];p+=1
            z|=(b&127)<<k;k+=7
            if b<128:break
        poly.append((z>>1)^-(z&1))
    shift=seg.bit_length()-1
    v=poly[-1]
    for a in reversed(poly[:-1]):v=a+((v*i)>>shift)
    bit=i*w
    if w:
        off=p+bit//8
        byte_count=(bit%8+w+7)//8
        val=(int.from_bytes(data[off:off+byte_count],'little')>>(bit%8))&((1<<w)-1)
    else:val=0
    return (base+(((v+(1<<(q-1)))>>q) if q else v)+lo+val)&U32

def trial(words,step):
    payload=[];offs=[];prev=0
    for i in range(0,len(words),step):
        b=encoded_segment(words[i:i+step],step)
        offs.append(prev)
        payload.append(b);prev+=len(b)
    table=struct.pack('<HH',step,len(offs))+struct.pack('<%dH'%len(offs),*offs)+b''.join(payload)
    bs,ns=struct.unpack_from('<HH',table)
    for i,word in enumerate(words):
        start=4+2*ns+struct.unpack_from('<H',table,4+2*(i//bs))[0]
        assert one(table,start,i%bs,bs)==word,(step,i)
    return table,max(map(len,payload)),max(b[0]>>5 for b in payload)

def generate(source, dest):
    text=Path(source).read_text()
    out=Path(dest)
    out.mkdir(parents=True,exist_ok=True)
    for name in ('a','e'):
        _,_,words=input_words(text,name)
        data,_,_=trial(words,128)
        (out/f'p_{name}.h').write_text('static const unsigned char p_'+name+'[]={\n'+
            ''.join(','.join(str(x) for x in data[i:i+32])+',\n' for i in range(0,len(data),32))+'};\n')
        print(f'Chicago FAST32 {name}: {len(words)} bit-exact coefficients in {len(data)} B')

if __name__=='__main__':
    generate(sys.argv[1],sys.argv[2])
