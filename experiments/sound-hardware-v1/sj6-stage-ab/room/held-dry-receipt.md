# Held dry and independent Room tail on send-close

Test-only continuation of [the scoped room A/B](README.md). Apply the delta below to the diagnostic tail.inc, not to product firmware; compile with compile.txt. The sign of dry and both sends vary in the test, the source is the same unmodified stock room for the sealed wet tail, and gate closes and reopens twice on fractional positions. All 49,152 Float32 words matched the independently added dry plus compact wet reference. The preceding zero/fresh-owner and toggle witnesses still pass. This fixes the dry attenuation in the diagnostic binary without changing the steady-on room or adding state; cost, tail retirement and full Type II tone are not proven.

```diff
--- a/tail.inc
+++ b/tail.inc
@@ -130,8 +130,8 @@
             if(++s->tick==SJ6_ROOM_STRIDE){s->tick=0;sj6_room_tick(s);}
             if(s->mix>0.f){
                 float t=(float)s->tick*(1.f/SJ6_ROOM_STRIDE);
-                l+=(s->last[0]+t*(s->wet[0]-s->last[0])-l)*s->mix;
-                r+=(s->last[1]+t*(s->wet[1]-s->last[1])-r)*s->mix;
+                l+=(s->last[0]+t*(s->wet[0]-s->last[0])-(s->enable?l:0.f))*s->mix;
+                r+=(s->last[1]+t*(s->wet[1]-s->last[1])-(s->enable?r:0.f))*s->mix;
             }
         }
         if(s->widen)r=sj6_fatten_tick(s,r);
```

```python
import ctypes as C
import numpy as np, sys, hashlib, json
from pathlib import Path
D=Path(sys.argv[1]); n=24576
bounds=[(6143,9971),(14211,16607)]
def f(name):
 l=C.CDLL(str(D/name)); fn=l.render_room_toggle
 p=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
 fn.argtypes=[C.c_uint,p,p,C.c_uint];fn.restype=C.c_int
 return fn
stock=f('c.so'); candidate=f('tail.so')
x=np.zeros((2,n),dtype='f4')
x[0,0]=.25;x[1,37]=-.125;x[1,10006]=.25;x[0,16611]=-.31
dry=np.zeros_like(x)
for a,b in bounds:
 t=np.arange(b-a)
 dry[0,a:b]=np.float32(.3)*np.sin(np.float32(.19)*t)
 dry[1,a:b]=np.float32(.2)*np.cos(np.float32(.67)*t)
ref=x.copy(); got=x+dry
assert stock(0,ref[0],ref[1],n)==1
assert candidate(1,got[0],got[1],n)==1
want=ref+dry
np.testing.assert_array_equal(got,want)
assert got.tobytes()==want.tobytes()
print(json.dumps(dict(frames=n,signed_words=2*n,closed_intervals=bounds,held_dry_and_retained_wet_exact=True,signed_sha=hashlib.sha256(got.tobytes()).hexdigest(),scope='full wet setting while active; unaffected dry plus matching compact wet tail when send closed; no installed parity or target cost')))
```

```json
{"frames": 24576, "signed_words": 49152, "closed_intervals": [[6143, 9971], [14211, 16607]], "held_dry_and_retained_wet_exact": true, "signed_sha": "6fa548eeb79de988c17f52180ec76109db3aab7ef9c5dc86cc54515a67c01844", "scope": "full wet setting while active; unaffected dry plus matching compact wet tail when send closed; no installed parity or target cost"}
```
