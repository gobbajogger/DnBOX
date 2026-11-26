"""Verify readbacks and compare equally partitioned SuperJ6 cutoff-step jobs."""
import hashlib
import json
from pathlib import Path
import sys
import numpy as np

AUTHORED = 0.3345831036567688
BINARY = "1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
CUE = "17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943"
JOBS = ("sj6-before-chord", "sj6-after-chord")
FRAMES = [49331,49587,70000]
TREATMENT = ([1.,1.,AUTHORED],[AUTHORED,1.,AUTHORED])
WINDOWS = {"before":(48500,49331),"onset":(49459,49587),"after-step":(49587,49715),
           "handoff":(49715,50003),"settled":(54000,69872),"restore":(70000,70256),
           "after-restore":(72500,81500)}
def params(rows):
    return {p["address"]:p["value"] for p in rows}

def require(ok,msg):
    if not ok:
        raise ValueError(msg)
def read_job(root,index):
    parent=root/JOBS[index]
    req=json.loads((parent/"request.json").read_text())
    plan=[(d["frame"],d["value"]) for d in req["cutoffSteps"]]
    require(req["id"]==JOBS[index] and req["profile"]=="dynamic" and plan==list(zip(FRAMES,TREATMENT[index])),"wrong submitted recipe")
    data={}
    for kind in ("physical-authored","physical-open"):
        for repeat in (False,True):
            name="native-warm-brainwave-osc2-live-authored-"+kind+("-repeat" if repeat else "")
            path=parent/name
            r=json.loads((path/"capture.json").read_text())
            x=np.load(path/"output.npy",allow_pickle=False)
            k=f"{parent.name}/{kind}/{repeat}"
            require(r["installed_au_sha256"]==BINARY and r["s4_input_sha256"]==CUE,k+" identity")
            require(x.shape==(2,148684) and x.dtype==np.float32 and np.isfinite(x).all(),k+" shape")
            require(r["physical_variant"]==kind and r["repeat"] is repeat,k+" arm")
            require(r["rate"]==44100 and r["warm_frames"]==16384 and sum(r["buffers"])==148684,k+" clock")
            require(hashlib.sha256(x.astype("<f4").tobytes()).hexdigest()==r["pcm_sha256"],k+" hash")
            base=params(r["parameters"])
            require(r["parameters"]==r["parameter_readback_after"] and base["7"]==AUTHORED,k+" state")
            steps=r["physical_steps"]
            values=TREATMENT[index] if kind=="physical-open" else [AUTHORED]*3
            previous=AUTHORED
            require(len(steps)==3,k+" step count")
            for step,frame,value in zip(steps,FRAMES,values):
                require((step["frame"],step["previous"],step["raw"],step["changed"])==(frame,previous,value,["7"] if previous!=value else []),k+" unexpected step")
                require(params(step["full_readback"])==base|{"7":value},k+" live drift")
                previous=value
            data[kind,repeat]=(r,x.astype(np.float64))
    return data
def db(x,y):
    return round(float(20*np.log10(max(1e-25,np.linalg.norm(x))/max(1e-25,np.linalg.norm(y)))),8)

def metrics(base,other,s,e):
    a,b=base[:,s:e],other[:,s:e]
    return {"level_db":db(b,a),"difference_dbr":db(b-a,a)}
def spectral_peaks_around_settled_chord(a,b,s,e):
    # This is a glide/chord window, not a stationary isolated note. Select the
    # strongest separated peaks of the first arm, not a guessed harmonic grid.
    x,y=a[0,s:e],b[0,s:e]
    w=np.hanning(e-s)
    grid=np.fft.rfftfreq(1<<17,1/44100)
    ref=np.abs(np.fft.rfft(x*w,1<<17))
    alt=np.abs(np.fft.rfft(y*w,1<<17))
    local=np.flatnonzero((ref[1:-1]>ref[:-2])&(ref[1:-1]>ref[2:]))+1
    freqs=[i for i in local if 40<grid[i]<1000]
    peaks=[]
    for i in sorted(freqs,key=lambda q:-ref[q]):
        if all(abs(grid[i]-grid[j])>=5 for j in peaks):
            peaks.append(i)
        if len(peaks)==8:
            break
    return [{"frequency_hz":round(float(grid[i]),3),"after_vs_before_db":db(alt[i-3:i+4],ref[i-3:i+4])} for i in peaks]

def run(root):
    before,after=[read_job(root,i) for i in range(2)]
    ctrl=("physical-authored",False)
    active=("physical-open",False)
    reference=before[ctrl][0]
    for group in (before,after):
        for r,_ in group.values():
            require(all(r[key]==reference[key] for key in ("schedule","buffers","parameters","osc2_static_writes")),"timing or initial state diverged")
    x,y=before[active][1],after[active][1]
    result={}
    for label,(s,e) in WINDOWS.items():
        result[label]={"bounds":[s,e],"after_vs_before":metrics(x,y,s,e)}
        for name,group in zip(("before","after"),(before,after)):
            result[label][name+"_vs_no_op"]=metrics(group[ctrl][1],group[active][1],s,e)
            result[label][name+"_repeat"]=metrics(group[active][1],group["physical-open",True][1],s,e)
    return {"windows":result,"settled_spectral_peaks":spectral_peaks_around_settled_chord(x,y,*WINDOWS["settled"])}
if __name__=="__main__":
    print(json.dumps(run(Path(sys.argv[1])),indent=2,sort_keys=True))
