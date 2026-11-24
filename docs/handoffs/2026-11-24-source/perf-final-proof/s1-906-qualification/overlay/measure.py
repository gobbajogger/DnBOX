import collections, ctypes as C, hashlib, json, pathlib, struct, subprocess, sys, tempfile, uuid, zipfile, zlib
import numpy as np
sys.dont_write_bytecode=True
r=pathlib.Path(sys.argv[1]).resolve()
def auth(p,h):
    b=p.read_bytes(); assert hashlib.sha256(b).hexdigest()==h; return b
up=r/'Synth one(1).zip'
auth(up,'68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4')
blob=auth(r/'s1-fix-data/s1.pack','c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44')
lab=r/'DnBOX-preset-compression-lab(1).zip'
with tempfile.TemporaryDirectory() as td:
    d=pathlib.Path(td)
    with zipfile.ZipFile(lab) as z:
        for n in ('bench_corpus.py','benchmark.py'):
            (d/n).write_bytes(z.read(n))
    sys.path.insert(0,str(d))
    from bench_corpus import s1_load_zip
    rows, stats=s1_load_zip(up)
    assert len(rows)==906 and len(stats['banks'])==13 and not stats['missing_defaults']
    src=r/'full-fw/firmware/src/dsp/s1pack/s1pack.c'
    subprocess.run(['cc','-O2','-shared','-fPIC',src,'-o',d/'s1.so'],check=True)
    lib=C.CDLL(str(d/'s1.so'))
    cb_t=C.CFUNCTYPE(None,C.c_void_p,C.c_uint,C.c_float)
    lib.s1pack_read.argtypes=[C.c_char_p,C.c_uint,C.c_char_p,cb_t,C.c_void_p]
    lib.s1pack_programs.argtypes=[C.c_char_p,C.c_uint]
    assert lib.s1pack_programs(blob,len(blob))==694
    base=[]
    for n in range(694):
        q=[]
        cb=cb_t(lambda _,j,v:q.append((j,struct.unpack('<I',struct.pack('<f',v))[0])))
        assert lib.s1pack_read(blob,n,C.create_string_buffer(64),cb,None)==144
        base.append(tuple(q))

bybank=collections.defaultdict(list)
for p in rows: bybank[p.bank].append(p)
idx=collections.defaultdict(list)
for i,rec in enumerate(base): idx[rec].append(i)
ids=tuple(j for j,_ in base[0])
assert all(tuple(j for j,_ in b)==ids for b in base)
bwords=np.asarray([[v for _,v in b] for b in base],dtype='<u4')
catalog=[]; new=[]; match=set()
with zipfile.ZipFile(up) as z:
    for name in z.namelist():
        if not(name.startswith('Synth one/') and name.endswith('.json')): continue
        bank=pathlib.Path(name).stem
        authored=json.loads(z.read(name))
        ordered=sorted(range(len(authored)),key=lambda i:authored[i]['position'])
        assert len(ordered)==len(bybank[bank])
        out=[None]*len(authored); source=[None]*len(authored)
        for i,p in zip(ordered,bybank[bank]):
            a=authored[i]
            assert (p.name,p.pos,p.uid)==(a['name'],a['position'],str(a['uid']))
            rec=tuple((int(j),v) for j,v in p.fields)
            assert tuple(j for j,_ in rec)==ids
            if rec in idx: ref=idx[rec][0]; match.update(idx[rec])
            else: ref=694+len(new); new.append(tuple(v for _,v in rec))
            out[i]=(p.name,p.pos,p.uid,ref)
            source[i]=rec
        for row,rec in zip(out,source):
            ref=row[3]
            decoded=base[ref] if ref<694 else tuple(zip(ids,new[ref-694]))
            assert decoded==rec
        catalog.append((bank,out))
assert len(match)==693 and 43 not in match and len(set(new))==len(new)==213
# Names, original JSON array order (implicit by sequence), signed authored position and textual UID.
# Use uint16 lengths rather than forbidden delimiters; refs address old or new words only, never emit UID as identity.
def field(s):
    b=s.encode('utf-8');return struct.pack('<H',len(b))+b
def uid_field(s):
    try: u=uuid.UUID(s)
    except ValueError: return b'\0'+field(s)
    if s==str(u): return b'\1'+u.bytes
    if s==str(u).upper(): return b'\2'+u.bytes
    return b'\0'+field(s)
def cab(rows):
    return b''.join(struct.pack('<iH',pos,ref)+field(name)+uid_field(uid) for name,pos,uid,ref in rows)
def uncat(b):
    def string():
        nonlocal i
        n,=struct.unpack_from('<H',b,i);i+=2;t=b[i:i+n].decode();i+=n;return t
    i=0;out=[]
    while i<len(b):
        pos,ref=struct.unpack_from('<iH',b,i);i+=6
        name=string(); typ=b[i]; i+=1
        if typ:
            assert typ in (1,2)
            uid=str(uuid.UUID(bytes=b[i:i+16])); i+=16
            if typ==2: uid=uid.upper()
        else: uid=string()
        out.append((name,pos,uid,ref))
    return out
def deflate(b):
    c=zlib.compressobj(9,zlib.DEFLATED,-15)
    out=c.compress(b)+c.flush()
    assert zlib.decompress(out,-15)==b
    return out
# Delta stores one old ordinal, 18-byte changed bitmap and exact little-endian XORs.
deltas=[]; changed=[]
for rec in new:
    x=np.bitwise_xor(bwords,np.asarray(rec,dtype='<u4'))
    # Estimate changed cost by nonzero XOR bytes, breaking ties by fewer changed words.
    words=np.count_nonzero(x,axis=1)
    scores=np.count_nonzero(x.view('u1'),axis=1)*145+words
    i=int(np.argmin(scores)); mask=np.packbits(x[i]!=0,bitorder='little').tobytes()
    deltas.append(struct.pack('<H',i)+mask+x[i][x[i]!=0].tobytes())
    changed.append(int(words[i]))
raw=[struct.pack('<144I',*x) for x in new]
def decode_delta(row):
    i,=struct.unpack_from('<H',row)
    mask=np.unpackbits(np.frombuffer(row[2:20],dtype='u1'),bitorder='little').astype(bool)
    v=bwords[i].copy();v[mask]^=np.frombuffer(row[20:],dtype='<u4')
    return tuple(map(int,v))
assert [decode_delta(d) for d in deltas]==new
catblocks=[]
for bank,entries in catalog:
    for i in range(0,len(entries),16):
        block=entries[i:i+16]; encoded=deflate(cab(block))
        assert uncat(zlib.decompress(encoded,-15))==block
        catblocks.append((len(encoded),len(cab(block))))
# One actual-sized envelope: 4 magic + version/counts etc (16 B), 2 SHA256s (64),
# 4 total bytes + 4 CRC-32 (8); per bank name, row count (2), first block (2).
# 8 per block for independent offset + encoded length + decoded length.
header=88
bankdir=sum(len(field(bank))+4 for bank,_ in catalog)
catbytes=sum(c for c,_ in catblocks)+8*len(catblocks)
wholecat=deflate(b''.join(cab(ents) for _,ents in catalog))
print('old',len(blob),'old matched',len(match),'upload',len(rows),'new',len(new),'banks',len(catalog))
print('catalog block16',len(catblocks),'with_8B_dir',catbytes,'max_decoded',max(n for _,n in catblocks),'whole comparator',len(wholecat))
print('uid encodings',dict(collections.Counter(uid_field(uid)[0] for _,rows in catalog for _,_,uid,_ in rows)))
print('header',header,'bank_dir',bankdir,'raw new',sum(map(len,raw)),'delta raw',sum(map(len,deltas)),'changed min/avg/max',min(changed),sum(changed)/len(changed),max(changed))
for label,items in [('raw',raw),('old-delta',deltas)]:
    for s in (1,8,16,213):
        blocks=[]
        for i in range(0,len(items),s):
            subset=items[i:i+s]
            data=b''.join((struct.pack('<H',len(v))+v if label=='old-delta' else v) for v in subset)
            encoded=deflate(data); decoded=zlib.decompress(encoded,-15); output=[]
            while decoded:
                if label=='raw': output.append(struct.unpack('<144I',decoded[:576]));decoded=decoded[576:]
                else:
                    n,=struct.unpack_from('<H',decoded);output.append(decode_delta(decoded[2:2+n]));decoded=decoded[n+2:]
            assert output==new[i:i+s]
            blocks.append((len(encoded),len(data)))
        byte=sum(n+8 for n,_ in blocks)
        print(label,s,'body+dir',byte,'max decoded',max(n for _,n in blocks),'overlay bytes',byte+catbytes+bankdir+header)
