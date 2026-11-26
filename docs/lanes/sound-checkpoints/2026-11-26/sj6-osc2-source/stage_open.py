"""Create code-only direct AUv3 OSC2 static/open capture from pinned dry host."""
import argparse,hashlib,tarfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('out',type=Path);args=p.parse_args()
A=Path('sj6-direct-stage/SJ6-Brainwave-Direct-AUv3-Fanin.tgz')
assert hashlib.sha256(A.read_bytes()).hexdigest()=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
with tarfile.open(A) as t:
 f=t.extractfile('main.swift').read().decode()
 src=t.extractfile('s4-inputs.json').read()
def ch(a,b):
 global f
 assert f.count(a)==1,a
 f=f.replace(a,b)
ch('native-warm-brainwave-fanin-','native-warm-brainwave-osc2-static-')
ch('let sequence = ["all","osc1-only","osc2-only","sub-only","all-repeat","osc1-only-repeat","osc2-only-repeat","sub-only-repeat"].map{($0,"authored")}','let sequence = ["osc2-only","osc2-only-repeat"].map{($0,"authored")}')
ch('All eight direct renders complete','Both directly matched OSC2 static captures complete')
ch(' let active=params()', ''' let stageWrites:[(AUParameterAddress,Float)] = [
   (87,0),(95,0),(99,0), // no animated filter/oscillator modulation
   (129,1),(131,0),(136,0),(137,0) // full cutoff, zero track, contour and velocity
 ]
 for (id,value) in stageWrites {
   guard let p=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing static control "+String(id))}
   p.setValue(value,originator:nil)
 }
 try require(wait(6,{stageWrites.allSatisfy{id,value in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==value}}),"static writes not reached")
 _=wait(0.35,{false})
 let active=params()''')
ch(' try require(Set(pre.keys.filter{pre[$0] != now[$0]}) == Set(mutes+(factor==1 ? [] : [master.identifier])),"unintended parameter drift")', ''' let stageIDs=Set(stageWrites.map{String($0.0)})
 let stageNames=Set(active.filter{stageIDs.contains($0["address"] as? String ?? "")}.compactMap{$0["identifier"] as? String})
 try require(stageNames.count==stageWrites.count,"static write map")
 try require(Set(pre.keys.filter{pre[$0] != now[$0]}).isSubset(of:Set(mutes).union(stageNames)),"unintended parameter drift")
 for id in mutes {try require(now[id]==0,"mute changed")}
 for (id,value) in stageWrites {
   let node=active.first(where: {($0["address"] as? String)==String(id)})
   try require((node?["value"] as? Double)==Double(value),"static control drift")
 }''')
ch('&& avDiff.count<10', '&& avDiff.filter{!stageNames.contains($0)}.count<10')
ch('"ablation_arm":mode,','"osc2_static_writes":Dictionary(uniqueKeysWithValues:stageWrites.map{(String($0.0),Double($0.1))}),"ablation_arm":mode,')
args.out.mkdir(exist_ok=False)
(args.out/'main.swift').write_text(f);(args.out/'s4-inputs.json').write_bytes(src)
