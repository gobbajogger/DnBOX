"""Generate direct AUv3 step/no-op physical cutoff timing harness."""
from pathlib import Path
import hashlib,tarfile,sys
if len(sys.argv)!=2:raise SystemExit(__doc__)
root=Path(sys.argv[1]);root.mkdir(exist_ok=False)
src=Path('sj6-direct-stage/SJ6-Direct-AUv3-Physical-Cutoff.tgz')
assert hashlib.sha256(src.read_bytes()).hexdigest()=='9581a0d4b230b6242723d12f2b58fd09d73f1474ea65f85cf9de3db28cf5f09b'
with tarfile.open(src) as t:
 s=t.extractfile('main.swift').read().decode()
 config=t.extractfile('s4-inputs.json').read()
def ch(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
ch('native-warm-brainwave-osc2-physical-', 'native-warm-brainwave-osc2-live-')
ch('let stageWrites = commonStageWrites + [(7,variant=="physical-open" ? Float(1) : authoredPhysical)]','let stageWrites = commonStageWrites + [(7,authoredPhysical)]')
ch(' while y[0].count<frames {',''' var physicalSteps:[[String:Any]]=[]
 let steps:[Int]=[24576,61440]
 var step=0
 while y[0].count<frames {''')
ch('  let pos=y[0].count','''  let pos=y[0].count
  if step<steps.count && pos==steps[step] {
    let value:Float = variant=="physical-open" && step==0 ? 1 : authoredPhysical
    let before=params()
    physical.setValue(value,originator:nil)
    try require(wait(6,{physical.value.bitPattern==value.bitPattern}),"physical write did not reach its readback")
    // No audio sample clock advances. Let the same observer interval elapse
    // in both the no-op and changed arm before checking the complete map.
    _=wait(0.15,{false})
    try require(engine.manualRenderingSampleTime==AVAudioFramePosition(pos),"control write advanced render clock")
    let observed=params()
    let preMap=Dictionary(uniqueKeysWithValues:before.map{($0["address"] as! String,$0["value"] as! Double)})
    let observedMap=Dictionary(uniqueKeysWithValues:observed.map{($0["address"] as! String,$0["value"] as! Double)})
    let changed=Set(preMap.keys.filter{preMap[$0] != observedMap[$0]})
    try require(changed.isSubset(of:Set(["7"])) && observedMap["7"]==Double(value),"unexpected control change")
    physicalSteps.append(["frame":pos,"raw":Double(value),"previous":preMap["7"]!,"changed":changed.sorted(),"full_readback":observed])
    step+=1
  }''')
ch('  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)','''  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum,
            step<steps.count ? steps[step]-pos : quantum)''')
ch(' try require(post==now,"post-render parameter drift")',' try require(step==steps.count && post==now,"post-render parameter drift")')
ch(' "physical_variant":variant,',' "physical_variant":variant,"physical_steps":physicalSteps,')
(root/'main.swift').write_text(s)
(root/'s4-inputs.json').write_bytes(config)
