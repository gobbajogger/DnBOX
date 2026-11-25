"""Compare reversible field-conditioned escape lanes against the shared DNB2 trial."""
from pathlib import Path
import sys,json,math
root=Path(__file__).resolve().parent
sys.path.insert(0,str(root/'source'))
shared=root/'shared_dict.py'
code=shared.read_text().replace("sys.path.insert(0,str(pathlib.Path(__file__).parent/'sources/source'))","")
a=code.index("    hh=struct.Struct('<4s8sHHII')")
b=code.index('    extents={}',a)
code=code[:a]+'''    hh=struct.Struct('<4s8sHHIIB')
    def anchor(tbl,values):
        candidates=[(n,i) for i,n in zip(tbl.syms,tbl.lengths) if i>=0]
        return values[min(candidates)[1]] if candidates else 0
    def encode_bank(e,bid,rows):
        lm={fs:i for i,(et,fs) in enumerate(layouts)if et==e}
        hs=headers(rows,lm)
        esc=[]
        pulse=Bits()
        for i,p,n,fs in rows:
            for f,w in fs:
                t=tables[e,f]; idx=wm.get(w,-1)
                if idx not in t.codes:idx=-1
                if idx==-1:esc.append((f,w))
                t.emit(pulse,idx)
        choices=[]
        for mode in range(4):
            sequence=esc if mode==0 else sorted(esc,key=lambda z:z[0])
            words_out=[]; last={}
            for f,w in sequence:
                if mode<2:
                    words_out.append(w)
                else:
                    if f not in last:last[f]=anchor(tables[e,f],words)
                    old=last[f];last[f]=w
                    if mode==2:words_out.append(w^old)
                    else:
                        u=(w-old)&0xffffffff
                        n=(u^0x80000000)-0x80000000
                        words_out.append((n<<1)^(n>>31))
            exc=b''.join(bytes((w>>shift)&255 for w in words_out)for shift in range(0,32,8))
            raw=hs+exc+bytes(pulse.data)
            payload=hh.pack(b'XBD3',bid,len(rows),len(hs),len(esc),len(raw),mode)+codec(name,raw)
            choices.append(payload+p32.pack(zlib.crc32(payload)))
        return min(choices,key=len)
    def decode_bank(e,bid,payload):
        assert p32.unpack_from(payload,len(payload)-4)[0]==zlib.crc32(payload[:-4])
        mg,bankid,m,hn,en,un,mode=hh.unpack_from(payload)
        assert mg==b'XBD3' and bankid==bid and 0<=mode<4
        decoded=codec(name,payload[hh.size:-4],un)
        head=unheader(decoded[:hn],m)
        exc=decoded[hn:hn+4*en]
        packed=[sum(k<<s for k,s in zip(lanes,range(0,32,8)))for lanes in zip(*(exc[i*en:(i+1)*en]for i in range(4)))]
        rd=Reader(decoded[hn+4*en:]);rebuild=[];todo=[]
        for i,p,n,l in head:
            label,fs=layout[l];assert label==e
            row=[]
            for f in fs:
                ref=read_symbol(tt[e,f],rd)
                if ref<0:
                    todo.append((f,row,len(row)));row.append((f,None))
                else:row.append((f,exact[ref]))
            rebuild.append((i,p,n,row))
        assert len(todo)==len(packed)==en
        if mode:todo.sort(key=lambda x:x[0])
        last={}
        for (f,row,idx),v in zip(todo,packed):
            if mode<2:w=v
            else:
                if f not in last:last[f]=anchor(tt[e,f],exact)
                old=last[f]
                w=v^old if mode==2 else (old+((v>>1)^-(v&1)))&0xffffffff
                last[f]=w
            row[idx]=(f,w)
        return [(i,p,n,tuple(row))for i,p,n,row in rebuild]
'''+code[b:]
before="        assert decode_bank(e,bid,payload)==rows,b"
after="""        got=decode_bank(e,bid,payload)
        if got!=rows:
            for i,(x,y) in enumerate(zip(got,rows)):
                if x!=y:
                    raise AssertionError((len(rows),i,payload[hh.size-1],
                      x[:3]==y[:3],len(x[3]),len(y[3]),
                      [(k,xf==yf)for k,(xf,yf)in enumerate(zip(x[3],y[3]))if xf!=yf][:8]))
            raise AssertionError((len(got),len(rows)))
"""
assert code.count(before)==1
code=code.replace(before,after,1)
# Alternate representations are evaluated after the outer codec. Banks whose
# positions have gaps always retain a format that can represent every position.
dense_head=('--dense-head' in sys.argv)
if dense_head:
    at=code.index('    def encode_bank(e,bid,rows):',code.index("    hh=struct.Struct('<4s8sHHIIB')"))
    code=code[:at]+'''    def head_options(rows,lm):
        ps=[p for i,p,n,fs in rows]
        ls=bytes(lm[tuple(f for f,v in fs)]for i,p,n,fs in rows)
        ns=[n for i,p,n,fs in rows]
        cs=bytes(len(n)for n in ns)
        names=b''.join(ns)
        ids=b''.join(i for i,p,n,fs in rows)
        yield 0,headers(rows,lm)
        yield 1,struct.pack('<%dH'%len(ps),*ps)+ls+cs+ids+names
        if ps==list(range(ps[0],ps[0]+len(ps))):
            yield 2,p16.pack(ps[0])+ls+cs+ids+names
            prefix=[]; suffix=[]; previous=b''
            for n in ns:
                k=0
                for x,y in zip(previous,n):
                    if x!=y: break
                    k+=1
                prefix.append(k);suffix.append(n[k:]);previous=n
            yield 3,p16.pack(ps[0])+ls+cs+bytes(prefix)+ids+b''.join(suffix)
    def read_head(data,m,hmode):
        if not hmode:return unheader(data,m)
        if hmode==1:
            ps=struct.unpack_from('<%dH'%m,data);z=m*2
        else:
            ps=range(p16.unpack_from(data)[0],p16.unpack_from(data)[0]+m);z=2
        ls=data[z:z+m];cs=data[z+m:z+2*m];z+=2*m
        prefixes=data[z:z+m] if hmode==3 else bytes(m)
        if hmode==3:z+=m
        ids=[data[p:p+8]for p in range(z,z+8*m,8)];z+=8*m
        rows=[];previous=b''
        for i,p,c,pre,l in zip(ids,ps,cs,prefixes,ls):
            assert pre<=min(len(previous),c)
            n=previous[:pre]+data[z:z+c-pre];z+=c-pre
            rows.append((i,p,n,l));previous=n
        assert z==len(data)
        return rows
'''+code[at:]
    code=code.replace('''            raw=hs+exc+bytes(pulse.data)
            payload=hh.pack(b'XBD3',bid,len(rows),len(hs),len(esc),len(raw),mode)+codec(name,raw)
            choices.append(payload+p32.pack(zlib.crc32(payload)))''','''            for hmode,hs in head_options(rows,lm):
                raw=hs+exc+bytes(pulse.data)
                payload=hh.pack(b'XBD3',bid,len(rows),len(hs),len(esc),len(raw),mode+4*hmode)+codec(name,raw)
                choices.append(payload+p32.pack(zlib.crc32(payload)))''',1)
    code=code.replace('0<=mode<4','0<=mode<16',1)
    code=code.replace('        head=unheader(decoded[:hn],m)\n        exc=', '        head=read_head(decoded[:hn],m,mode>>2)\n        mode &= 3\n        exc=',1)
# Alternative first-production native identity, intentionally distinct from DNB2.
# Exact original app UID plus old proposed DNB2 SID remain in the audited
# Library/browser catalog, not necessarily in resident bank payload.
implicit_id=('--implicit-id' in sys.argv)
if implicit_id:
    assert dense_head
    code=code.replace('    def head_options(rows,lm):','''    def record_id(bid,p):
        return sid('resident-factory-v1',bid.hex(),p)
    def head_options(rows,lm,bid):''',1)
    code=code.replace('    def read_head(data,m,hmode):','''        if all(i==record_id(bid,p)for i,p,n,fs in rows):
            if ps==list(range(ps[0],ps[0]+len(ps))):
                yield 4,p16.pack(ps[0])+ls+cs+names
            yield 5,struct.pack('<%dH'%len(ps),*ps)+ls+cs+names
    def read_head(data,m,hmode,bid):''',1)
    code=code.replace('        if hmode==1:','        if hmode in (1,5):',1)
    code=code.replace('        ids=[data[p:p+8]for p in range(z,z+8*m,8)];z+=8*m','''        if hmode<4:
            ids=[data[p:p+8]for p in range(z,z+8*m,8)];z+=8*m
        else:ids=[record_id(bid,p)for p in ps]''',1)
    code=code.replace('head_options(rows,lm):','head_options(rows,lm,bid):',1)
    code=code.replace('read_head(decoded[:hn],m,mode>>2)','read_head(decoded[:hn],m,mode>>2,bid)',1)
    code=code.replace('0<=mode<16','0<=mode<24',1)
alias=('--aliases' in sys.argv)
if alias:
    cut=code.index('def frequent(')
    common,code=code[:cut],code[cut:]
    code=code.replace('    md=bytes(model);','''    eq=collections.Counter((e,tuple(fs))for(e,b),(bid,rows)in banks.items()for _i,_p,_n,fs in rows)
    duplicates=tuple(sorted(k for k,n in eq.items()if n>1))
    assert len(duplicates)<256
    alias_code={k:i+1 for i,k in enumerate(duplicates)}
    model+=bytes([len(duplicates)])
    for e,fs in duplicates:
        shape=layouts.index((e,tuple(f for f,w in fs)))
        model+=bytes([shape])+b''.join(p32.pack(w)for f,w in fs)
    md=bytes(model);''',1)
    code=code.replace('    assert off==len(q)','''    alias_rec=[None]
    cnt=q[off];off+=1
    for _ in range(cnt):
        i=q[off];off+=1
        e,fs=layout[i];vals=struct.unpack_from('<'+'I'*len(fs),q,off);off+=4*len(fs)
        alias_rec.append((e,tuple(zip(fs,vals))))
    assert off==len(q) and tuple(alias_rec[1:])==duplicates''',1)
    code=code.replace('''        pulse=Bits()
        for i,p,n,fs in rows:
            for f,w in fs:''','''        pulse=Bits()
        encoded_aliases=[]
        for i,p,n,fs in rows:
            a=alias_code.get((e,tuple(fs)),0)
            encoded_aliases.append(a)
            if a:continue
            for f,w in fs:''',1)
    code=code.replace('                raw=hs+exc+bytes(pulse.data)','                raw=hs+bytes(encoded_aliases)+exc+bytes(pulse.data)',1)
    code=code.replace('        exc=decoded[hn:hn+4*en]','''        alias_ids=decoded[hn:hn+m]; hn+=m
        exc=decoded[hn:hn+4*en]''',1)
    code=code.replace('''        for i,p,n,l in head:
            label,fs=layout[l];assert label==e
            row=[]''','''        for a,(i,p,n,l) in zip(alias_ids,head):
            label,fs=layout[l];assert label==e
            if a:
                al,af=alias_rec[a]
                assert al==e and tuple(f for f,w in af)==fs
                rebuild.append((i,p,n,af));continue
            row=[]''',1)
    code=common+code
use_journal=("--journal" in sys.argv)
if use_journal:
    from shared_page_trial import sim
    code=code.replace('def frequent(banks,name,minimum):','def frequent(banks,name,minimum,witness):',1)
    needle='    extents[target]=encode_bank(e,bid,fresh)'
    assert code.count(needle)==1
    code=code.replace(needle,needle+'\n    witness(z,dict(extents),banks,target,old,extents[target],decode_bank,fresh)',1)
if implicit_id:
    # Prove an ordinal-changing edit with the old resident reference still
    # exact. It must fall back to explicit row IDs, not silently change ID.
    a='    extents[target]=encode_bank(e,bid,fresh)'
    code=code.replace(a,a+'''
    move_id,move_pos,move_name,move_words=fresh[0]
    new_pos=next(p for p in range(65535,0,-1) if all(p!=r[1] for r in fresh))
    moved=[(move_id,new_pos,move_name,move_words),*fresh[1:]]
    check=encode_bank(e,bid,moved)
    assert check[hh.size-1]<16
    assert decode_bank(e,bid,check)==moved
    print(json.dumps(dict(moved_explicit_bytes=len(check),same_position_bytes=len(extents[target]),stable_id_preserved=True)),flush=True)''',1)
scope={'__file__':str(shared),'__name__':'escape_trial'}
exec(compile(code,str(shared),'exec'),scope)
j=scope['load_j6'](sys.argv[1])[0]
s=scope['s1_load_zip'](sys.argv[2])[0]
assert (len(j),len(s),sum(len(p.fields) for p in j+s))==(640,906,221013)
for family,ps in ([('all',j+s)] if use_journal else [('all',j+s),('superj6',j),('synthone',s)]):
 banks=scope['grouped'](ps)
 if implicit_id:
  prior=banks
  archive={}
  newer={}
  sid=scope['sid']
  for b,(bank,rows) in prior.items():
   seen=set()
   aa=[]
   for old,position,name,words in rows:
    # Stable within the fixed official bank source. A moved/custom row
    # must keep its old tag with explicit row IDs, never regenerate.
    key=sid('resident-factory-v1',bank.hex(),position)
    assert key not in archive and position not in seen
    archive[key]=old;seen.add(position)
    aa.append((key,position,name,words))
   newer[b]=(bank,aa)
  assert len(archive)==len(ps)
  restored={b:(bank,[(archive[key],p,n,w)for key,p,n,w in rr])for b,(bank,rr)in newer.items()}
  assert restored==prior
  banks=newer

 for cut in ((2,) if use_journal else (2,4,8)):
  result=scope['frequent'](banks,'lzma',cut,*([sim] if use_journal else []))
  p=result['shared_bytes']+result['bank_bytes']
  # At most three padding bytes per record, before the independent A/B dirs.
  padded=(p+3*(len(banks)+1)+4095)//4096
  print(json.dumps(dict(family=family,minimum=cut,records=len(ps),
    raw=p,standalone_sector_bytes=4096*(result['bank_sectors']+result['shared_sectors']),
    shared_sector_upper_bytes=4096*padded,checked=result['checked_replace_and_remove'])),flush=True)
