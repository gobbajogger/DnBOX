"""Validate four native AU renders and score the live physical-cutoff step."""
import hashlib
import json
from pathlib import Path
import sys
import numpy as np

root=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent
data={}
authored=0.3345831036567688
binary="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
cue="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943"
def require(ok,message):
    if not ok: raise ValueError(message)
def params(source):
    return {p["address"]:p["value"] for p in source}
for arm in ("physical-authored","physical-open"):
    for repeat in (False,True):
        folder=root/("native-warm-brainwave-osc2-live-authored-"+arm+("-repeat" if repeat else ""))
        j=json.loads((folder/"capture.json").read_text())
        x=np.load(folder/"output.npy",allow_pickle=False)
        require(j["physical_variant"]==arm and j["repeat"] is repeat,"identity")
        require(j["installed_au_sha256"]==binary and j["s4_input_sha256"]==cue,"identity checksum")
        require(x.shape==(2,148684) and x.dtype==np.float32 and np.isfinite(x).all(),"PCM type")
        require(hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()==j["pcm_sha256"],"PCM hash")
        require(j["parameters"]==j["parameter_readback_after"] and j["rate"]==44100 and j["warm_frames"]==16384,"parameter or clock drift")
        opened=arm=="physical-open"
        base=params(j["parameters"])
        expect=[(24576,authored,1.,["7"]),(61440,1.,authored,["7"])] if opened else [(24576,authored,authored,[]),(61440,authored,authored,[])]
        require([(p["frame"],p["previous"],p["raw"],p["changed"]) for p in j["physical_steps"]]==expect,"stage events differ")
        for step in j["physical_steps"]:
            require(params(step["full_readback"])==base|{"7":step["raw"]},"drift during live step")
        data[arm,repeat]=j,x.astype(np.float64)
a,x=data["physical-authored",False]
b,y=data["physical-open",False]
for j,_ in data.values():
    require(all(j[k]==a[k] for k in ("schedule","buffers","parameters","osc2_static_writes")),"unmatched capture inputs")
def db(a,b):
    return round(float(20*np.log10(max(np.linalg.norm(a),1e-25)/max(np.linalg.norm(b),1e-25))),8)
out={}
windows={"pre-step":(22000,24576),"first-step-128":(24576,24704),"held-after-step":(30000,37634),"next-chord":(53971,61440),"restore-128":(61440,61568),"held-after-restore":(64000,80000)}
for label,(i,j) in windows.items():
    u,v=x[:,i:j],y[:,i:j]
    out[label]={"bounds":[i,j],"level_db":db(v,u),"difference_dbr":db(v-u,u),"repeat_dbr":{arm:db(data[arm,True][1][:,i:j]-pcm,pcm) for arm,pcm in [("physical-authored",u),("physical-open",v)]}}
print(json.dumps(out,indent=2,sort_keys=True))
