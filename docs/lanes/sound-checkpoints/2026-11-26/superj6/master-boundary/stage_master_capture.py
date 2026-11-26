"""Generate source-only authorized offline public-AU capture on Mac; does not run it."""
from pathlib import Path
import tarfile,hashlib,sys
if len(sys.argv)!=2:
 raise SystemExit("stage_master_capture.py /path/to/new-stage-dir")
d=Path(sys.argv[1]);d.mkdir(exist_ok=False)
p=Path('recovered-sound/SJ6-Brainwave-Warm-Main-Sub.native.private.tgz')
assert hashlib.sha256(p.read_bytes()).hexdigest()=='9db19abc7c56183e745e1f3de150559b319dabda4817ebd30af9a9db356c4b53'
with tarfile.open(p) as t:
 s=t.extractfile('CaptureWarmBrainwaveStems.swift').read().decode()
 cfg=t.extractfile('s4-inputs.json').read()
assert hashlib.sha256(cfg).hexdigest()=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
def sub(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
sub(' let arm=CommandLine.arguments[1]', ''' try require(CommandLine.arguments.count==3,"usage: CaptureMaster main-only[-repeat]|sub-only[-repeat] authored|half|quarter")
 let arm=CommandLine.arguments[1]
 let masterMode=CommandLine.arguments[2]
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}''')
sub('native-warm-brainwave-\\(arm)', 'native-warm-brainwave-master-\\(masterMode)-\\(arm)')
sub(' let active=params()', ''' guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let active=params()''')
sub(' == Set(mutes),"unintended parameter drift")',' == Set(mutes+(factor==1 ? [] : [master.identifier])),"unintended parameter drift")')
sub('"ablation_arm":mode,"repeat":repeatArm,','"ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),')
(d/'CaptureMaster.swift').write_text(s)
(d/'s4-inputs.json').write_bytes(cfg)
