"""Compose already-verified AUv3 capture profiles into the fixed bench host."""
import argparse
from pathlib import Path

PROFILES = ("master", "fanin", "static", "physical", "dynamic")
p = argparse.ArgumentParser()
for profile in PROFILES:
    p.add_argument(f"--{profile}", type=Path, required=True)
p.add_argument("--host", type=Path, required=True)
p.add_argument("--gain", type=Path, required=True)
args=p.parse_args()
prelude = None
modules=[]
for profile in PROFILES:
    text=getattr(args,profile).read_text()
    head,tail=text.split("\nfunc runJ6(",1)
    if prelude is None:
        prelude=head
    elif head!=prelude:
        raise ValueError(f"{profile}: unexpected helper revision")
    body=tail.split("\nlet workRoot = Bundle.main.bundleURL",1)[0]
    signature='_ arm:String,_ masterMode:String,_ av:AVAudioUnit) throws {'
    if not body.startswith(signature):
        raise ValueError(f"{profile}: unexpected capture signature")
    body=body.replace(signature,signature.replace(") throws",",_ workRoot:URL,_ plan:[CutoffStep]?) throws"),1)
    if profile=="dynamic":
        before=' let steps:[Int]=[24576,61440]'
        after=' let steps:[CutoffStep]=plan ?? [CutoffStep(frame:24576,value:1),CutoffStep(frame:61440,value:Float(0.3345831036567688))]'
        assert body.count(before)==1
        body=body.replace(before,after)
        assert body.count('steps[step]')==2
        body=body.replace('pos==steps[step]','pos==steps[step].frame')
        body=body.replace('steps[step]-pos','steps[step].frame-pos')
        body=body.replace('variant=="physical-open" && step==0 ? 1 : authoredPhysical','variant=="physical-open" ? steps[step].value : authoredPhysical')
        expect=' try require(step==steps.count && post==now,"post-render parameter drift")'
        # The job schema requires the last treatment step to restore #7.
        assert body.count(expect)==1
    modules.append(f"func capture_{profile}("+body)
print(prelude)
print("\n".join(modules))
print(args.gain.read_text())
print(args.host.read_text())
