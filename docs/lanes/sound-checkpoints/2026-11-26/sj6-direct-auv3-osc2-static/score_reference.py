"""Validate exact identity/readback and score repeatability for static-open AU."""
import hashlib
import json
from pathlib import Path
import numpy as np

def read(repeat):
    path = Path("native-warm-brainwave-osc2-static-authored-osc2-only"+("-repeat" if repeat else ""))
    state = json.loads((path/"capture.json").read_text())
    samples = np.load(path/"output.npy",allow_pickle=False)
    assert state["installed_au_sha256"]=="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
    assert state["s4_input_sha256"]=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943"
    assert state["osc2_static_writes"]=={"87":0,"95":0,"99":0,"129":1,"131":0,"136":0,"137":0}
    assert state["parameters"]==state["parameter_readback_after"]
    controls={p["address"]:p["value"] for p in state["parameters"]}
    assert all(controls[k]==v for k,v in state["osc2_static_writes"].items())
    assert (controls["3"],controls["0"],controls["209"],controls["130"])==(1,.3150142729282379,0,.4141378700733185)
    assert not any(controls[str(k)] for k in (1,11,16,17,22,23,24,28,40,41,59,103,107,108,109,201,202))
    assert samples.shape==(2,148684) and state["rate"]==44100 and state["warm_frames"]==16384 and state["repeat"] is repeat
    assert np.isfinite(samples).all() and not samples[:,:16384].any()
    assert hashlib.sha256(samples.astype("<f4").tobytes()).hexdigest()==state["pcm_sha256"]
    return state,samples.astype(np.float64)

identity,a=read(False)
repeat,b=read(True)
assert all(identity[k]==repeat[k] for k in ("schedule","buffers","parameters"))
out={}
for label,lo,hi in [("attack",16384,18432),("held",24576,37634),("before-chord",49331,49459),("near-crossing",49715,49843),("crossing",49875,50003)]:
    x,y=a[:,lo:hi],b[:,lo:hi]
    out[label]={"level_db":float(20*np.log10(np.linalg.norm(y)/np.linalg.norm(x))),
                "null_dbr":float(20*np.log10(np.linalg.norm(y-x)/np.linalg.norm(x)))}
print(json.dumps(out,indent=2,sort_keys=True))
