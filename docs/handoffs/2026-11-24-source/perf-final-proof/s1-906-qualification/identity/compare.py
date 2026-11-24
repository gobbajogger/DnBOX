"""Private content witness for the exact 694 built-in and optional uploaded 906."""
import argparse, collections, ctypes as C, hashlib, io, json, os, pathlib, struct
import subprocess, sys, tarfile, tempfile, zipfile

BASE='73749f55d17a0643f1ddd072114172d29eecc2ae'
UPLOAD='68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4'
sha=lambda b:hashlib.sha256(b).hexdigest()
binary=lambda fields:b''.join(struct.pack('<II',int(i),w) for i,w in fields)

def main():
    p=argparse.ArgumentParser()
    p.add_argument('repo',type=pathlib.Path);p.add_argument('lab',type=pathlib.Path)
    p.add_argument('--upload',type=pathlib.Path);p.add_argument('--private-catalog',type=pathlib.Path)
    a=p.parse_args()
    if a.upload and sha(a.upload.read_bytes())!=UPLOAD: p.error('upload SHA256 does not match the pinned 906 input')
    if bool(a.upload)!=bool(a.private_catalog): p.error('upload and private-catalog are required together')
    with tempfile.TemporaryDirectory() as t:
        return run(a,pathlib.Path(t))

def run(a,d):
    paths=('firmware/src/dsp/s1pack','firmware/src/dsp/synthone_controls_data.inc',
           'tools/compile-synthone-factory.py','tools/pack_synthone_factory.py',
           'tests/fixtures/synthone-factory/frontend','assets/factory_banks/synthone')
    archive=subprocess.check_output(['git','-C',a.repo,'archive',BASE,*paths])
    with tarfile.open(fileobj=io.BytesIO(archive)) as z: z.extractall(d,filter='data')
    subprocess.run([sys.executable,d/'tools/compile-synthone-factory.py','--out',d],
                   check=True,stdout=subprocess.DEVNULL)
    sys.dont_write_bytecode=True
    sys.path.insert(0,str(d/'tools'))
    import pack_synthone_factory as pack
    programs,banks,order=pack.load_programs(d/'ORIGINAL-PRESET-COMMANDS.json')
    blob,_=pack.pack(programs,banks,order)
    src=d/'firmware/src/dsp/s1pack/s1pack.c'
    subprocess.run(['cc','-O2','-shared','-fPIC',src,'-o',d/'s1.so'],check=True)
    lib=C.CDLL(str(d/'s1.so'))
    CB=C.CFUNCTYPE(None,C.c_void_p,C.c_uint,C.c_float)
    lib.s1pack_programs.argtypes=[C.c_char_p,C.c_uint]
    lib.s1pack_read.argtypes=[C.c_char_p,C.c_uint,C.c_char_p,CB,C.c_void_p]
    assert lib.s1pack_programs(blob,len(blob))==len(programs)==694
    index=collections.defaultdict(list)
    for bank,start,size,position in banks:
        for i in range(start,start+size):
            decoded=[]
            cb=CB(lambda _,j,v: decoded.append((j,struct.unpack('<I',struct.pack('<f',v))[0])))
            name=C.create_string_buffer(64)
            assert lib.s1pack_read(blob,i,name,cb,None)>0
            assert name.value.decode()==programs[i][0] and len(decoded)==144
            raw=binary(decoded)
            assert raw==b''.join(struct.pack('<If',j,v) for j,v in programs[i][1])
            index[raw].append(dict(bank=bank,position=position+i-start,ordinal=i+1,name=name.value.decode()))
    print('baseline',BASE,'S1PK SHA256',sha(blob),'size',len(blob),
          'decoded 694 x 144 ordered words;',len(index),'distinct content sequences;',
          'duplicates',694-len(index))
    if not a.upload:
        print('BLOCKED exact uploaded Synth one(1).zip missing; no corpus overlap inferred')
        return
    with zipfile.ZipFile(a.lab) as lab:
        for f in ('bench_corpus.py','benchmark.py'):
            (d/f).write_bytes(lab.read(f))
    sys.path.insert(0,str(d))
    from bench_corpus import s1_load_zip
    rows,stats=s1_load_zip(a.upload)
    assert len(rows)==906 and len(stats['banks'])==13 and not stats['missing_defaults']
    # Host lab orders by position. Retain serialized array indices too.
    with zipfile.ZipFile(a.upload) as up:
        original={}
        for member in up.namelist():
            if member.startswith('Synth one/') and member.endswith('.json'):
                bank=pathlib.Path(member).stem
                assert bank not in original
                original[bank]=list(enumerate(json.loads(up.read(member))))
    stream=[];used=collections.Counter(); by_uid=collections.defaultdict(set); hits=0
    book={}
    for bank in original:
        rawrows=sorted(original[bank],key=lambda pair:pair[1]['position'])
        mapped=[r for r in rows if r.bank==bank]
        assert len(mapped)==len(rawrows)
        result=[None]*len(rawrows)
        for (array_index,obj),r in zip(rawrows,mapped):
            assert (str(obj['uid']),obj['position'],obj['name'])==(r.uid,r.pos,r.name)
            raw=binary(r.fields)
            rev=sha(raw);used[rev]+=1;by_uid[r.uid].add(rev)
            matches=index.get(raw,[])
            hits+=bool(matches)
            result[array_index]=dict(name=r.name,array_index=array_index,position=r.pos,uid=r.uid,
                                     content_sha256=rev,exact_factory_matches=matches)
        book[bank]=result
    summary=dict(upload_entries=len(rows),matching_entries=hits,not_in_694=len(rows)-hits,
                 duplicate_content_entries=sum(n-1 for n in used.values()),
                 duplicate_uid_occurrences=stats['duplicate_uids'],
                 uid_groups_with_distinct_content=sum(len(v)>1 for v in by_uid.values()))
    out=dict(build=BASE,source_pack_sha256=sha(blob),upload_sha256=UPLOAD,summary=summary,banks=book)
    fd=os.open(a.private_catalog,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
    with os.fdopen(fd,'w') as f:
        json.dump(out,f,ensure_ascii=False,indent=2);f.write('\n')
    print(json.dumps(summary))

if __name__=='__main__':main()
