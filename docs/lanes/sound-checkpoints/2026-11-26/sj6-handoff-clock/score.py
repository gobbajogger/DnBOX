"""Compare physical phase at absolute source callback prep boundaries; no fit or shift."""
from pathlib import Path
import sys, json, hashlib
import numpy as np
p=Path(sys.argv[1])
def points(f):
    out={}
    for line in (p/f).read_text().splitlines():
        a=line.split()
        if a[0] != "phase": continue
        key=(int(a[1]),int(a[2]))
        v=tuple(float.fromhex(t) for t in a[4:8])
        assert key not in out or out[key]==v
        out[key]=v
    return out
older=points("provider-phase.txt"); new=points("release-glide-phase.txt")
orig=json.loads((p/"native-pending.private.json").read_bytes())
assert orig["oracle_sha"]=="6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850"
base=np.fromfile(p/"provider.private.f32","<f4").reshape(3,148684)
cand=np.fromfile(p/"release-glide.private.f32","<f4").reshape(3,148684)
sha=lambda a:hashlib.sha256(a.tobytes()).hexdigest()
assert sha(base)=="0c8a3900405610432e8786d77011ae89f48c7c4a8b95bf89729c03e7a9f81756"
assert base[:,:49459].tobytes()==cand[:,:49459].tobytes()
rows=[]
for l,c in enumerate(orig["cases"]):
    assert c["lead"]==11025 and c["who"]==("oscSampler1","oscSampler2")[l]
    prep={q[0]+16384:q for q in c["prep"]}
    arm=[]
    for f in range(49459,49875,16):
        b=older[l,f]; t=new[l,f]; o=prep[f]
        assert t[1]==o[6],(l,f,t[1],o[6])
        assert t[3]==b[3]
        arm.append(dict(frame=f,reference_phase=o[6],stock_phase=b[1],candidate_phase=t[1],
                        stock_hz=b[0]*44100,candidate_hz=t[0]*44100))
    assert len(arm)==26
    rows.append(dict(owner=c["who"],pending_exact=len(arm),pending=arm,
                      reference_first_postcommit_phase=prep[49891][6],
                      stock_first_postcommit_phase=older[l,49891][1],
                      candidate_first_postcommit_phase=new[l,49891][1]))
print(json.dumps(dict(scope="Typed Main source only. Authentic current receiver with independently old-projected exact controls, no full Apple graph or newer terminal scoring.",arm=rows,
       provider_base_sha=sha(base),provider_candidate_sha=sha(cand), all_three_pre_note_transition_tapes_exact=True),indent=2))
