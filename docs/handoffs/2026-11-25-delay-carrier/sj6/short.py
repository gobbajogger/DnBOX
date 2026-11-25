from compare_import import variants
import numpy as np
r=np.random.default_rng(6589)
cases=0
for detent in (0,1):
 for fb in (0,64,126,127):
  for mix in (0,1,63,126,127):
   x=r.uniform(-.25,.25,size=(2,96000)).astype(np.float32)
   y=x.copy()
   a=variants['base'].compact(detent,fb,mix,*x,96000)
   b=variants['center'].compact(detent,fb,mix,*y,96000)
   assert a==b==1
   assert x.tobytes()==y.tobytes()
   cases+=1
print(f"{cases} cases, {cases*96000*2:,} float samples bit exact")
