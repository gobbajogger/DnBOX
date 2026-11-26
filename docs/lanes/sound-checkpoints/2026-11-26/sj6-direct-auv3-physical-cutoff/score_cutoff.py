"""Verify and contrast direct SuperJ6 captures whose only control delta is #7."""
import hashlib
import json
from pathlib import Path
import numpy as np

def read(variant, repeat):
    path = Path(f"native-warm-brainwave-osc2-physical-authored-physical-{variant}"+("-repeat" if repeat else ""))
    j = json.loads((path/"capture.json").read_text())
    x = np.load(path/"output.npy", allow_pickle=False)
    def check(test, label):
        if not test:
            raise ValueError(f"{variant}/{repeat}: {label}")
    params = {v["address"]: v["value"] for v in j["parameters"]}
    check(j["physical_variant"]=="physical-"+variant and j["repeat"] is repeat, "wrong arm")
    check(j["installed_au_sha256"]=="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257", "wrong binary")
    check(j["s4_input_sha256"]=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943", "wrong cue")
    check(x.dtype==np.float32 and x.shape==(2,148684) and np.isfinite(x).all(), "bad PCM")
    check(hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()==j["pcm_sha256"],"PCM digest mismatch")
    check(j["parameters"]==j["parameter_readback_after"],"readback drift")
    check(j["rate"]==44100 and j["warm_frames"]==16384,"wrong timing")
    check(j["osc2_static_writes"]=={"7":(.3345831036567688 if variant=="authored" else 1),"87":0,"95":0,"99":0,"129":1,"131":0,"136":0,"137":0},"wrong armed controls")
    check(all(params[k]==v for k,v in j["osc2_static_writes"].items()),"wrong live controls")
    return params,x.astype(np.float64),j

raw={(v,r):read(v,r) for v in ("authored","open") for r in (False,True)}
p,x,j=raw["authored",False]
q,y,k=raw["open",False]
if set(p)!=set(q) or [key for key in p if p[key]!=q[key]]!=["7"]:
    raise ValueError("controls other than #7 changed")
for (variant, _), (params, _, receipt) in raw.items():
    if any(receipt[key]!=j[key] for key in ("schedule","buffers")) or params != raw[variant,False][0]:
        raise ValueError("schedule, buffer split or repeat parameters changed")

def db(a,b):
    return round(float(20*np.log10(max(np.linalg.norm(a),1e-20)/max(np.linalg.norm(b),1e-20))),9)

out={}
for label,i,h in [("attack",16384,18432),("held",24576,37634),("pending",49459,49875),("commit",49875,50003),("after",53971,81434)]:
    a,b=x[:,i:h],y[:,i:h]
    out[label]={"open_vs_authored_db":db(b,a),"difference_dbr":db(b-a,a),"repeat_dbr":{v:db(raw[v,True][1][:,i:h]-z,z) for v,z in [("authored",a),("open",b)]}}
i,h=24576,37634
t=np.arange(i,h)/44100
w=np.hanning(h-i)
harmonic={}
for n in range(1,7):
    e=w*np.exp(-2j*np.pi*n*32.636*t)
    harmonic[str(n)]=round(float(20*np.log10(abs(y[0,i:h]@e)/abs(x[0,i:h]@e))),9)
print(json.dumps({"windows":out,"held_harmonic_level_delta_db":harmonic},indent=2,sort_keys=True))
