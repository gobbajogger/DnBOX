#!/usr/bin/env python3
"""Compare two saved, independently loaded original AU authored takes."""
import hashlib
import io
import json
import sys
import tarfile
import numpy as np

def rms(x):
    return float(np.sqrt(np.mean(np.square(x))))

def db(x):
    return float(20*np.log10(max(x, 1e-15)))

def read(archive, program, arm):
    name = f"./program-{program}-{arm}"
    meta = json.load(archive.extractfile(name+"/capture.json"))
    raw = archive.extractfile(name+"/output.npy").read()
    y = np.load(io.BytesIO(raw), allow_pickle=False)
    assert y.dtype == np.dtype("<f4") and np.isfinite(y).all()
    assert y.shape == (2, meta["frames"])
    assert hashlib.sha256(y.tobytes()).hexdigest() == meta["pcm_sha256"]
    assert meta["active"] == meta["after"] and sum(meta["blocks"]) == meta["frames"]
    return meta, y.astype(np.float64)

def score(path):
    with tarfile.open(path,"r:gz") as archive:
        for program in range(3):
            ma, a = read(archive,program,"authored")
            mb, b = read(archive,program,"authored-repeat")
            for field in ("active","cue_sha256","schedule","rate","component",
                          "component_version","wanted","authored"):
                assert ma[field] == mb[field], field
            dry_meta, dry = read(archive,program,"dry")
            assert dry_meta["schedule"]==ma["schedule"] and dry_meta["cue_sha256"]==ma["cue_sha256"]
            events = sorted({e["frame"] for e in ma["schedule"] if e["action"]=="off"})
            print(ma["wanted"]["program"])
            rate = ma["rate"]
            marks = sorted({e["frame"] for e in ma["schedule"]})
            for n, frame in enumerate(events,1):
                stop = next((i for i in marks if i > frame), ma["frames"])
                for start, end in ((0,16),(16,50),(50,200),(200,500)):
                    left, right = (round(rate*ms/1000)+frame for ms in (start,end))
                    right = min(right, stop)
                    if left >= right:
                        continue
                    last = 1000*(right-frame)/rate
                    v,w = a[:,left:right], b[:,left:right]
                    s = rms(v-w)
                    ref = rms(v)
                    rel = db(s)-db(ref)
                    level = db(rms(w))-db(ref)
                    dr = db(rms(dry[:,left:right]))
                    # Report absolute floor with every ratio. Dry has no repeat.
                    err = f"{db(s):.2f} dBFS / {rel:.2f} dBr" if s else "bit-exact"
                    print(f"{n}:{start}-{last:.1f} ms authored {db(ref):.2f} dBFS, "
                          f"repeat error {err}, "
                          f"level {level:+.2f} dB; dry {dr:.2f} dBFS")

if __name__ == "__main__":
    score(sys.argv[1])
