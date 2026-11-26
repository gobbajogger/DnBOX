"""Validate native public-control sweeps and compare unnormalized held levels."""
import hashlib,json,sys
from pathlib import Path
import numpy as np
root=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent
binary="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
uid="5A84A4E4-B9C5-41C3-853E-BB4D43D48850"
faders={"response-f125":.125,"response-f250":.25,"response-f500":.5,"response-f1000":1}
velocities={"response-v32":32,"response-v64":64,"response-v87":87,"response-v127":127}
data={}
def db(r):return float(20*np.log10(r))
def read(f):
    j=json.loads(f.read_text());x=np.load(f.parent/"output.npy",allow_pickle=False)
    name,repeat=j["ablation_arm"],j["repeat"]
    p={row["address"]:row["value"] for row in j["parameters"]}
    assert name in faders or name in velocities or name=="both-muted"
    assert j["preset_name"]=="Jupiter Sine" and j["s4_pinned_identity"]["program_uid"]==uid and j["installed_au_sha256"]==binary
    assert j["component_version"]==67072 and j["master_mode"]=="authored" and p["0"]==0.44618064165115356
    assert j["rate"]==44100 and j["warm_frames"]==16384 and x.shape==(2,148684) and x.dtype==np.float32 and np.isfinite(x).all()
    assert j["schedule"]==[{"frame":16384,"note":69,"velocity":velocities.get(name,87),"action":"on"},{"frame":60484,"note":69,"velocity":0,"action":"off"}]
    assert p["1"]==0 and p["3"]==faders.get(name,0 if name=="both-muted" else 1) and p["205"]==int(name not in velocities)
    assert j["parameters"]==j["parameter_readback_after"] and all(p[k]==v for k,v in j["forced_controls"].items())
    assert all(p[str(k)]==0 for k in (11,16,17,22,23,24,40,41,59,87,91,95,99,103,107,108,109,201,202))
    assert hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()==j["pcm_sha256"]
    assert not x[:,:16384].any()
    return (name,repeat),(j,p,x.astype(float))
for cm in sorted(root.glob("native-jupiter-sine-gain-*/capture.json")):
    key,value=read(cm);assert key not in data
    data[key]=value
expect={(k,False) for k in (*faders,*velocities,"both-muted")}|{("response-v64",True)}
assert data.keys()==expect
base,bp,y=data["both-muted",False]
assert not y.any()
for j,p,x in data.values():
    assert j["buffers"]==base["buffers"]
    assert {k:v for k,v in p.items() if k not in ("3","205")}=={k:v for k,v in bp.items() if k not in ("3","205")}
out={"both_muted_peak":0.0,"cases":{}}
for name,rep in sorted(data):
    x=data[name,rep][2]-y
    if name=="both-muted":continue
    ref=data[("response-f1000" if name in faders else "response-v87"),False][2]-y
    predicted=db(faders[name] if name in faders else velocities[name]/87)
    row={}
    for label,lo,hi in (("early",24576,37634),("late",44000,59000)):
      signal=x[:,lo:hi];refsig=ref[:,lo:hi]
      norm=lambda a:np.linalg.norm(a)
      delta=db(norm(signal)/norm(refsig))
      row[label]={"native_vs_reference_db":delta,"source_receiver_expected_db":predicted,"relative_deviation_db":delta-predicted,
        "native_rms":np.sqrt(np.mean(signal*signal,axis=1)).tolist()}
    out["cases"][name+("-repeat" if rep else "")]=row
print(json.dumps(out,indent=2,sort_keys=True))
