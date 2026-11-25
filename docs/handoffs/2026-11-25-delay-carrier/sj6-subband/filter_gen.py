import numpy as np
from scipy.signal import firwin
f=(firwin(321, 1150, window=('kaiser', 6), fs=44100),
   firwin(321, [2850, 4020], pass_zero=False, window=('kaiser', 6), fs=44100))
for i,a in enumerate(f):
    print('static const float filter%d[321]={'%i,','.join(f'{v:.11e}f' for v in a),'};')
dec=np.expm1(np.log1p(255)*np.arange(128)/127)/255
print('static const float compand[128]={',','.join(f'{v:.11e}f' for v in dec),'};')
