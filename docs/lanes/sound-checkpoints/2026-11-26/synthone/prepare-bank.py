"""Reproduce private Synth One 906-bank test inputs without modifying source banks."""
import hashlib,json,re,struct,subprocess,sys,zipfile
from pathlib import Path
fw, archive, out=map(Path,sys.argv[1:])
assert not out.exists()
fw=fw.resolve()
out=out.resolve(); out.mkdir(mode=0o700)
assert hashlib.sha256(archive.read_bytes()).hexdigest()=="68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4"
dst=out/'assets/factory_banks/synthone'
dst.mkdir(parents=True)
import shutil
shutil.copytree(fw/'tests/fixtures/synthone-factory/frontend',out/'tests/fixtures/synthone-factory/frontend')
dsp=out/'firmware/src/dsp';dsp.mkdir(parents=True)
shutil.copyfile(fw/'firmware/src/dsp/synthone_controls_data.inc',dsp/'synthone_controls_data.inc')
tools=out/'tools';tools.mkdir()
shutil.copyfile(fw/'tools/compile-synthone-factory.py',tools/'compile-synthone-factory.py')
pack=(fw/'tools/pack_synthone_factory.py').read_text()
assert pack.count('len(programs) != 694')==1
(tools/'pack_synthone_factory.py').write_text(pack.replace('len(programs) != 694','len(programs) != 906'))
members=[]
with zipfile.ZipFile(archive) as src:
    for n in sorted(src.namelist()):
        if not n.startswith('Synth one/') or not n.endswith('.json'):continue
        assert n.count('/')==1
        b=src.read(n)
        s=Path(n).name
        q=json.loads(b)
        assert isinstance(q,list)
        members.append({'name':s,'sha256':hashlib.sha256(b).hexdigest(),'rows':len(q)})
        (dst/s).write_bytes(b)
assert len(members)==13 and sum(z['rows'] for z in members)==906
(dst/'MANIFEST.json').write_text(json.dumps({'files':members}))
private=out/'original-private'
subprocess.run([sys.executable,tools/'compile-synthone-factory.py','--out',private],check=True)
subprocess.run([sys.executable,tools/'pack_synthone_factory.py',private/'pack.private','--commands',private/'ORIGINAL-PRESET-COMMANDS.json'],check=True)
expect=private/'roundtrip.expect'
a=json.loads((private/'ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
x={(row['bank'],row['preset_index']):row for row in a}
with expect.open('wb') as o:
    for item in members:
        f=dst/item['name']
        for i,_ in sorted(enumerate(json.loads(f.read_text())),key=lambda v:v[1]['position']):
            z=x[(f.name,i)]
            label=z['name'].encode()
            assert len(label)<64 and len(z['commands'])==144
            o.write(bytes([len(label)])+label+struct.pack('<I',len(z['commands'])))
            for ident,v in z['commands']:o.write(struct.pack('<If',ident,v))
s=(fw/'tests/host/s1pack_roundtrip_test.c').read_text()
assert s.count('programs != 694')==1
s=s.replace('programs != 694','programs != 906').replace('../../firmware/src/','firmware/src/')
(out/'roundtrip.c').write_text(s)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror',f'-I{fw}',out/'roundtrip.c',
                fw/'firmware/src/dsp/s1pack/s1pack.c','-o',out/'roundtrip'],check=True)
result=subprocess.check_output([out/'roundtrip',private/'pack.private',expect],text=True)
assert result.splitlines()[-1]=='PASS'
print(result,end='')
checks={
'preset-commands.bin':'7747ee362a0cdbf33a3c9e8a0151356e9be273b8e88bbbe37b05d70e7bab7812',
'pack.private':'d5e003e878ab46d6051d6cff16be557fbdfde7fccaa9255e4217bb5ffb51fa43',
'roundtrip.expect':'99dbb13b3e5c60bb4b80935c2a30393c5b7ea6c3b5f08952b6eb6f11172a8d8e'}
for n,h in checks.items():assert hashlib.sha256((private/n).read_bytes()).hexdigest()==h
print('PASS: three independent recovered inputs match SHA-256')
