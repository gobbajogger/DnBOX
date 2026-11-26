import hashlib,io,json,pathlib,sys,tarfile
import numpy as np
root=pathlib.Path(sys.argv[2])
with tarfile.open(sys.argv[1]) as t:
    pre="./native/bare/"
    meta=json.load(t.extractfile(pre+"bare.json"))
    res=[]
    for m in meta["arms"]:
        if m["amount"]!=0 or m["policy"]!="m0": continue
        actual=np.load(io.BytesIO(t.extractfile(pre+m["file"]).read()),allow_pickle=False)
        assert hashlib.sha256(actual.tobytes()).hexdigest()==m["pcm_sha256"]
        assert m["before"]==m["after"] and m["before"]["3"]==m["before"]["4"]==m["before"]["15"]==0
        assert m["before"]["5"]==100 and m["before"]["14"]==float(np.float32(-.9))
        x=np.fromfile(root/f'{m["input_index"]}.f32',dtype="<f4")
        assert actual.shape==(2,len(x)) and sum(m["blocks"])==len(x) and x.max()>0
        last=state=np.float32(0); modeled=[]
        for cur in x:
            state=(cur-last)+np.float32(.995)*state
            last=cur
            modeled.append(state)
        d=actual.astype("float64")-modeled
        X=actual.astype("float64")-x
        def residual(e):
            return float(20*np.log10(np.sqrt(np.mean(e*e))/np.sqrt(np.mean(actual.astype("float64")**2))))
        res.append({"peak":m["peak_in"],"compared_stereo_frames":len(x),"dc_residual_db":residual(d),"bypass_residual_db":residual(X),"dc_max_abs":float(np.max(np.abs(d)))})
assert len(res)==12
print(json.dumps({"component":meta["component"],"component_version":meta["version"],"rate":meta["rate"],"results":res},indent=2))
