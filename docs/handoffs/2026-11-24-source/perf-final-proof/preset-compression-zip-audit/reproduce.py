"""Audit-only host reproduction from already checked lab sources. Prints aggregates only."""
import collections,hashlib,importlib,json,random,struct,sys,zlib
from pathlib import Path
root=Path(sys.argv[1]).resolve(); code=Path(sys.argv[2]).resolve()
assert code.is_dir()
sys.path.insert(0,str(code))
p=importlib.import_module('predictor_study')
c=importlib.import_module('cache_study')
b=importlib.import_module('benchmark')
expect=json.loads((code/'predictor-results.json').read_text())
jr,check=p.load_j6(root/'sj6-private-final/J6presets(2).zip')
jsha=hashlib.sha256((root/'sj6-private-final/J6presets(2).zip').read_bytes()).hexdigest()
assert jsha=='ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'
assert sorted(check['member_sha256'])==sorted(p.J6_HASHES)
# Reuse the authenticated real C-reader corpus loader; avoid lab's expanded
# archive loader and IPA comparison. Adapt installed typed IDs only.
loader=root/'perf-final-proof/sj6-combined-storage/measure-combined.py'
n={'__file__':str(loader)}
sys.argv=sys.argv[:2]
exec(compile(loader.read_text().split('def counters(subset):',1)[0],str(loader),'exec'),n)
sr=[]
for e,t,_uid,f,rs in n['banks']:
    if e: continue
    for i,(name,uid,fields) in enumerate(rs):
        sr.append(b.Preset('Synth One',t.decode(),f+i,name.decode(),tuple((str(k),v) for k,v in fields),uid.hex()))
assert len(sr)==694 and sum(len(x.fields) for x in sr)==99936
print('full pinned J6 archive + 12 member checks; installed Synth One independently checked through production C reader',len(jr),len(sr),flush=True)
save={}
def capture(progs):
    blob,man,rep=b.run(progs)
    save.update(blob=blob,man=man)
    return blob,man,rep
p.run=capture

def setup(blob):
    magic,ver,flags,n,ng,nf,nl,nb,io,so,crc=b.HEADER.unpack_from(blob)
    assert (magic,ver,flags)==(b'DNP1',1,0)
    gb,lb,ta,ga,pa=struct.unpack_from('<HHIII',blob,b.HEADER.size)
    pos=b.HEADER.size+16
    for j in range(nb):
        _ei,_idx,ln=struct.unpack_from('<HHB',blob,pos);pos+=5+ln
    pos+=2*n
    lays=[]
    for j in range(nl):
        ln=struct.unpack_from('<H',blob,pos)[0]
        lays.append(struct.unpack_from('<'+'H'*ln,blob,pos+2));pos+=2+2*ln
    assert pos==io
    gi=(ng+255)//256
    idx=[int.from_bytes(blob[o:o+3],'little') for o in range(io,so,3)]
    stream=blob[so:]
    r=b.Reader(stream);pt=b.read_table(r,False,256);nt=b.read_table(r,False,256)
    assert r.at==ta
    tabs=[b.read_table(r,True,ng) for _ in range(nf)]
    assert r.at==ga
    # Read-only independent checkpoint validation. Keep dictionary only in host
    # scratch; device runtime memory is measured below, not inferred from Python.
    keys=[]
    for j in range(gi):
        r=b.Reader(stream,idx[j]); k=r.get(5); v=r.get(32); keys.append(v)
        for _ in range(min(256,ng-j*256)-1):
            v+=r.eg(k);keys.append(v)
    return stream,pt,nt,tabs,keys,lays,idx[gi:],gb,lb,nf

def verify_checkpoints(ordered,refs,fmt):
    stream,pt,nt,tabs,keys,lays,idx,gb,lb,nf=setup(fmt['blob'])
    ids=[tuple(x) for x in fmt['man']['field_ids']]
    refs={(e,f):v for e,f,v,_,_ in refs}
    rng=random.Random(39)
    cases=list(range(len(idx)));rng.shuffle(cases)
    rows=words=0
    for k in cases:
        r=b.Reader(stream,idx[k]);name=b'';prev={};eng=None
        for prog in ordered[k*8:k*8+8]:
            if prog.engine!=eng:eng=prog.engine;prev={}
            pre=pt.read(r)
            assert pre<=len(name); out=bytearray(name[:pre])
            while (v:=nt.read(r)):out.append(v)
            name=bytes(out); assert name.decode()==prog.name
            layout=lays[r.get(lb)]; cur={}; got=[]
            for col in layout:
                e,f=ids[col];assert e==eng
                w=b.unkey(keys[tabs[col].read(r,gb)])
                if w==p.COPY:w=cur[refs[e,f]]
                elif w==p.PREV:w=prev[f]
                got.append((f,w));cur[f]=w
            assert tuple(got)==prog.fields
            rows+=1;words+=len(got);prev=cur
    sizes=[len(t.syms) for t in tabs]
    print('shuffled seek checkpoints, rows, exact words',len(idx),rows,words,
          'field IDs',nf,'largest table symbols',max(sizes),'sum table symbols',sum(sizes),
          'eager dictionary bytes',4*len(keys),'per field table index bytes at 24-bit',3*nf,flush=True)

# First reproduce the quoted exact value and actual byte signature. For current
# S1 use distinct named arms rather than claiming supplied expanded reproduction.
for label,ps in [('quoted J6',jr),('installed 694 S1',sr),('installed 694 S1 plus J6',jr+sr)]:
    for prev in (False,True):
        r,refs=p.predict(ps,prev)
        if label=='quoted J6':
            key='intra_and_previous' if prev else 'intra_only'
            assert r==expect['SuperJ6'][key],{k:(r[k],expect['SuperJ6'][key].get(k)) for k in r if r[k]!=expect['SuperJ6'][key].get(k)}
        print(label,'COPY+PREV' if prev else 'COPY',
              'base, add metadata, total, checkpoint, words, layouts, program count, refs, copy, prev',
              *(r[k] for k in ('pack_bytes','predictor_metadata_bytes','total_bytes','checkpoint_bytes','fields','layouts','programs','reference_fields','within_preset_references','previous_preset_references')),
              'stream-sha',r['sha256'],flush=True)
        verify_checkpoints(ps,refs,save)
print('expanded 906 S1 report by source only; no verified matching expanded input used; difference 212 programs',flush=True)
for label,ps in [('J6',jr),('installed current pair',jr+sr)]:
    cb,groups=c.catalog(ps)
    print(label,'actual DNC1 Huffman catalog (excluding UUID)',cb,'program UUID bytes',16*len(ps),'with literal UUIDs',cb+16*len(ps), 'bank count',len(groups),flush=True)
    if label=='J6': continue
    global_fields=sorted({(row.engine,f) for row in ps for f,_ in row.fields});gi={f:i for i,f in enumerate(global_fields)}
    banksizes=[]
    for (e,_),rs in groups.items():
        no_name=[b.Preset(x.engine,x.bank,x.pos,'',x.fields,x.uid,x.bank_uid) for x in rs]
        blob,m,rep=b.run(no_name)
        meta=struct.pack('<'+'H'*len(m['field_ids']),*(gi[tuple(x)] for x in m['field_ids']))
        body=meta+blob
        actual=struct.pack('<4sHHII',b'DNB1',len(meta)//2,len(rs),len(blob),zlib.crc32(body))+body
        banksizes.append((len(actual),e,len(rs)))
    top=sorted(banksizes,reverse=True)[:2]
    rounded=lambda size: (size+4095)//4096*4096
    print('current DNC1 + actual largest two bank payload bytes',cb,top,
          'logical with 64 directory',cb+sum(t[0] for t in top)+64,
          'sector-rounded',rounded(cb)+sum(rounded(t[0]) for t in top),
          'resident every independent bank plus catalog',cb+sum(t[0] for t in banksizes),flush=True)
