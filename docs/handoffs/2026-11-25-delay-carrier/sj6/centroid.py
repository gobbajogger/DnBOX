import json
import numpy as np
from compare_import import variants
cases=[]
for raw in (1,11,21,42,85,127):
  delay=np.float32(np.float32(1.5/127)*raw)*44100
  for name,d in variants.items():
    phase=[]
    for offset in range(256,384):
      pulse=np.zeros((2,round(delay)+2048),np.float32)
      pulse[0,offset]=.5
      stride=d.compact(raw,0,127,*pulse,pulse.shape[1])
      start=offset+int(delay)-2*stride
      end=offset+int(delay)+2*stride+1
      y=pulse[0,start:end].astype(float)
      assert y.sum()>0 and not pulse[1].any()
      phase.append(float(y@np.arange(start,end)/y.sum()-offset-delay))
    cases.append(dict(raw=raw,variant=name,stride=stride,mean=round(float(np.mean(phase)),4),worst_abs=round(float(np.max(np.abs(phase))),4)))
print(json.dumps(cases,indent=2))
