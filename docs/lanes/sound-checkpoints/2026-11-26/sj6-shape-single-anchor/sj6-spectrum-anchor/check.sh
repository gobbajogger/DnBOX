#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
d=$(mktemp -d)
trap 'rm -r "$d"' EXIT
python3 - "$d/anchor.json" <<'PY'
import json,sys
p={'key':69,'sample_rate':44100}
for q in ('sine','tri'):p[q]={'harmonics_h2_to_h18':[[-180,0]]*17,'normal_df1':[1.,0.,0.,0.,0.]}
json.dump(p,open(sys.argv[1],'w'))
PY
python3 generate.py "$d/anchor.json" >"$d/anchor-generated.private.h"
cc -shared -fPIC -O2 -std=c99 -ffp-contract=off -I"$d" -I../sj6-wavetable-scope -I../sj6-wave-carrier stage.c -lm -o "$d/test.so"
python3 - "$d/test.so" <<'PY'
import ctypes as c,sys,math
a=c.CDLL(sys.argv[1]).sj6_anchor_run
a.argtypes=c.POINTER(c.c_float),c.c_uint,c.c_float,c.POINTER(c.c_float),c.c_uint
m=44200
s=(c.c_float*17)()
x=(c.c_float*m)()
a(s,0,440/44100,x,m)
lo,hi=4410,22050
y=x[lo:hi]
assert abs(math.sqrt(sum(t*t for t in y)/len(y))-.5*1.2377909421920776/math.sqrt(2))<.001
a(s,0,220/44100,x,m)
assert s[0]>0 and all(math.isfinite(t) for t in x)
print('synthetic fixture only: compile, unity-H1 projection and bent stream smoke passed')
PY
