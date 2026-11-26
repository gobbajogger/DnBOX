from pathlib import Path
import tarfile,hashlib,sys
if len(sys.argv)!=2:raise SystemExit("stage.py destination-directory")
root=Path(sys.argv[1]);root.mkdir(exist_ok=False)
asset=Path('sj6-direct-stage/SJ6-Brainwave-Direct-AUv3-Master.tgz')
assert hashlib.sha256(asset.read_bytes()).hexdigest()=='82d0348669ebecb49245ee83fcb44c2a8576b9a2a5599e2f4591e671646bd5b8'
with tarfile.open(asset) as f:
 s=f.extractfile('main.swift').read().decode()
 conf=f.extractfile('DnBOX AUv3 Direct.app/Contents/Resources/s4-inputs.json').read()
assert hashlib.sha256(s.encode()).hexdigest()=='c71c21e84ac16bc1f4c08204876c31df0ae4279f88d59b03665fa57d0f39b577'
assert hashlib.sha256(conf).hexdigest()=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
def change(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
old='let options=["sub-only":["neonverbEnable","neonunison","neonpulseVolume","neonsawVolume"],"main-only":["neonverbEnable","neonunison","neonsubVolume"]]'
change(old,'''let common=["neonverbEnable","neonunison","neoncrushAmount"]
 let options=[
   "all":common,
   "osc1-only":common+["neonsawVolume","neonsubVolume"],
   "osc2-only":common+["neonpulseVolume","neonsubVolume"],
   "sub-only":common+["neonpulseVolume","neonsawVolume"]
 ]''')
change('"neonunison":1]','"neonunison":1,"neoncrushAmount":0.11941517889499664]')
change('avDiff.count<8','avDiff.count<10')
change('let sequence = [("main-only","authored"),("main-only","half"),("sub-only","authored"),("sub-only","half")]','let sequence = ["all","osc1-only","osc2-only","sub-only","all-repeat","osc1-only-repeat","osc2-only-repeat","sub-only-repeat"].map{($0,"authored")}')
change('All four direct renders complete','All eight direct renders complete')
change('native-warm-brainwave-master-', 'native-warm-brainwave-fanin-')
(root/'main.swift').write_text(s);(root/'s4-inputs.json').write_bytes(conf)
