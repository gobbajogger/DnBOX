import math
refs={
(1,57):(.18615142,.31469771,-6.055,{0:.261415880,2:.028714358,4:.010105331}),
(1,69):(.18624498,.31523198,-7.031,{0:.261529028,2:.028824454,4:.010209523}),
(2,57):(.16174422,.27732566,83.9448,dict(enumerate((.183155037,.091182767,.060354315,.044816675,.035400582,.029050600)))),
(2,69):(.16223339,.29290029,82.9688,dict(enumerate((.183234351,.091339746,.060585770,.045117916,.035765583,.029472154)))),
(3,57):(.17635630,.17970842,83.9448,{0:.228479539,2:.075289915,4:.044161009}),
(3,69):(.17672164,.18943292,82.9688,{0:.228578465,2:.075578662,4:.044616350}),
}
for name in ('ablation-0','post-0','post-3','exact-normal-3'):
 print(name)
 with open(name+'.txt') as f:
  for rr in f:
   wave,l,k,*d=map(float,rr.split())
   if l==2:continue
   a,b,phase,hs=refs[wave,k]; db=lambda x,y:20*math.log10(x/y)
   cp=math.degrees(math.atan2(-d[4], d[3]))
   h=[round(db(math.hypot(*d[3+2*j:5+2*j]),m),3) for j,m in hs.items()]
   print(f'{int(wave)} {int(k)} RMS {db(d[0],a):+.3f} dB peak {db(d[1],b):+.3f} dB h1phase {cp:+.3f} delta {cp-phase:+.3f} per_harmonic_dB {h}')
