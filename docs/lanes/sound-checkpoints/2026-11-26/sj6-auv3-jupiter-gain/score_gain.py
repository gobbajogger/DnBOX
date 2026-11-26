"""Authenticate native A4 recordings; preserve absolute level and time origin."""
import hashlib
import json
from pathlib import Path
import sys
import numpy as np
ROOT=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent
BINARY="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
UID="5A84A4E4-B9C5-41C3-853E-BB4D43D48850"
TAPE=[{"frame":16384,"note":69,"velocity":87,"action":"on"},{"frame":60484,"note":69,"velocity":0,"action":"off"}]
KNOWN={(a,m,r) for a in ("osc1-only","osc2-only") for m,r in (("authored",False),("half",False),("authored",True))}|{("both-muted","authored",False)}
def db(a,b):
    return float(20*np.log10(max(1e-25,np.linalg.norm(a))/max(1e-25,np.linalg.norm(b))))
raw={}
for f in ROOT.glob("native-jupiter-sine-gain-*/capture.json"):
    j=json.loads(f.read_text());x=np.load(f.parent/"output.npy",allow_pickle=False)
    key=(j["ablation_arm"],j["master_mode"],j["repeat"])
    assert key in KNOWN and key not in raw
    assert j["installed_au_sha256"]==BINARY and j["component_version"]==67072
    assert j["s4_pinned_identity"]["program_uid"]==UID and j["preset_name"]=="Jupiter Sine"
    assert j["calibration_cue"]=="jupiter-gain-A4-v1" and j["rate"]==44100 and j["schedule"]==TAPE
    assert j["warm_frames"]==16384 and x.shape==(2,148684) and x.dtype==np.float32 and np.isfinite(x).all()
    assert hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()==j["pcm_sha256"]
    assert j["parameters"]==j["parameter_readback_after"] and sum(j["buffers"])==x.shape[1]
    p={r["address"]:r["value"] for r in j["parameters"]}
    assert all(p[k]==v for k,v in j["forced_controls"].items())
    assert (p["1"],p["3"])=={"osc1-only":(1,0),"osc2-only":(0,1),"both-muted":(0,0)}[key[0]]
    assert p["0"]==(0.44618064165115356 if key[1]=="authored" else 0.22309032082557678)
    assert all(p[str(k)]==0 for k in (11,16,17,22,23,24,40,41,59,87,91,95,99,103,107,108,109,201,202))
    raw[key]=j,p,x.astype(np.float64)
assert raw.keys()==KNOWN
base,bp,mute=raw["both-muted","authored",False]
assert not mute.any()
for j,p,x in raw.values():
    assert {k:v for k,v in p.items() if k not in ("0","1","3")}=={k:v for k,v in bp.items() if k not in ("0","1","3")}
    assert j["buffers"]==base["buffers"] and not x[:,:16384].any()

out={"muted_peak":0.0,"reference_gain_not_normalized":True,"arms":{}}
for arm in ("osc1-only","osc2-only"):
    out["arms"][arm]={}
    for s,e,label in [(16384,18432,"attack"),(24576,37634,"held"),(44000,59000,"late-held")]:
        a,h,r=[raw[arm,p,q][2][:,s:e]-mute[:,s:e] for p,q in [("authored",False),("half",False),("authored",True)]]
        w=np.hanning(e-s)
        t=np.arange(s,e)/44100
        basis=w*np.exp(-2j*np.pi*440*t)
        f=np.fft.rfftfreq(1<<18,1/44100)
        region=np.flatnonzero((f>435)&(f<445))
        af=np.abs(np.fft.rfft(a[0]*w,1<<18))
        out["arms"][arm][label]={
        "bounds":[s,e],"full_rms":np.sqrt(np.mean(a*a,axis=1)).tolist(),"full_peak":np.max(np.abs(a),axis=1).tolist(),
        "half_rms":np.sqrt(np.mean(h*h,axis=1)).tolist(),"repeat_rms":np.sqrt(np.mean(r*r,axis=1)).tolist(),
        "half_vs_full_db":db(h,a),"repeat_vs_full_db":db(r,a),"repeat_residual_dbr":db(r-a,a),
        "full_dominant_hz":float(f[region[np.argmax(af[region])]]),
        "full_440_complex":[[float(z.real),float(z.imag)] for z in (a@basis)*(2/w.sum())]}
print(json.dumps(out,indent=2,sort_keys=True))
