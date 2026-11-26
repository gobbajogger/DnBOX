import json,math
import numpy as np
from scipy.signal import sosfilt,butter
import refs
from refs import run,sr
refs.N=sr*16
result={}
lengths=[p*80 for p in [97,101,103,107]]
result['published']=dict(combLengthsAt44100=lengths,liveCellsPerInput=sum(lengths),taps=408,monoOpsSample=802,
 models=[dict(decimation=d,wordBits=b,inputLanes=c,owners=2,roomHistoryPerOwner=sum(math.ceil(x/d) for x in lengths)*b*c//8,
  statePerOwnerIncludingOptimisticDigest=math.ceil(sum(math.ceil(x/d) for x in lengths)*b*c/8)+c*(4*10*2*4+4*2*4)+3*math.ceil(20/d)*c*2+16,
  totalTwoOwnersHistory=2*math.ceil(sum(math.ceil(x/d) for x in lengths)*b*c/8),
  opCallbacksOneOwner=math.ceil(512/d)*802*c,bandLimitHz=sr/2/d) for d in (1,2,4,8) for b in (4,8,16) for c in (1,2)],
 optimisticPackedTapFlashBytes=408*2, tenOctaveAndShelvesFloatCoefficients=4*(10*5+3)*4,sparseTablesOmittedExtraForExtraStereoBanks=True)
lens=[1721,1901,2063,2213,2399,2579,2789,2939,3109,3271,3449,3643,3833,4027,4211,4397]
result['VFDN2020']=dict(
 publishedVN15OpsSample=1296,publishedOVNOpsSample=1776,publishedEqualFdnOpsSample=848,
 publishedCoreCells=sum(lens),publishedCoreFourBitFullRate=math.ceil(sum(lens)/2),
 quarterRateCoreCells=sum(math.ceil(x/4) for x in lens),
 quarterRateExtraReadInputAndOutputCellsUpperForSingleStereoFedCore=(16+2)*math.ceil(sr/4*.01),
 quarterRateFourBitCoreAndIndependentHistories=(sum(math.ceil(x/4) for x in lens)+(16+2)*math.ceil(sr/4*.01))/2,
 holdoutQualified=False)
def power(a):return float(np.sum(a*a))
def db(x):return 10*math.log10(max(x,1e-40))
result['source']=[]
for fb in (.47,.72,.89,.96,.985):
 l=run(fb);r=run(fb,1)
 anti=l-r; common=l+r; assert anti.any()
 z=dict(fb=fb,cold_anti_nonzero=bool(anti.any()),antiToCommonLateDb=db(power(anti[4410:])/power(common[4410:])),bands=[])
 for fl,fh in ((160,900),(1000,4000),(5000,16000)):
  h=sosfilt(butter(2,(fl,fh),btype='bandpass',fs=sr,output='sos'),l,axis=0)
  k0=int(.1*sr);s=h[k0:]
  if not s.any():continue
  q=np.sum(s*s,axis=1); v=np.cumsum(q[::-1])[::-1]/sum(q)
  first=lambda threshold: next((i for i,x in enumerate(v) if x<threshold),len(v))
  energy=(sum(q)/len(q))
  z['bands'].append(dict(hz=[fl,fh], late_band_dbfs=db(energy), fromImpulseToE30s=.1+first(1e-3)/sr,fromImpulseToE60s=.1+first(1e-6)/sr,
      capturedIRSeconds=16,edgeNearWindowEnd=bool(first(1e-6)/sr>15.5)))
 result['source'].append(z)
print(json.dumps(result,indent=2))
