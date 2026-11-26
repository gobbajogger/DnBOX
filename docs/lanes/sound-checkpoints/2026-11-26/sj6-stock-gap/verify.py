import numpy as np, hashlib, pathlib, sys,json
p,arm,v,pin=sys.argv[1:]
r=np.fromfile(p,dtype='<f4').reshape(-1,2).T
assert r.shape==(2,148684)
s=hashlib.sha256(r.tobytes()).hexdigest()
assert s==json.loads(pathlib.Path(pin).read_text())['compact_sha']['center-'+arm]
print(v,arm,s)
