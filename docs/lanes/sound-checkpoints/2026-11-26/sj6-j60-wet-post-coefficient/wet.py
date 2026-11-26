from pathlib import Path
import sys
i,o,m=sys.argv[1:]
s=Path(i).read_text()
def rep(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new)
if m in ('phase','phase-delay','all'):
 rep('const float hz=mode==3u?9.2f:mode==2u?.82f:.48f;','const double hz=mode==3u?9.2:mode==2u?.82:.48;')
 rep('float phase=s->phase+s->rate*(2.f*hz+(s->phase<0.f?-.048f:.048f))*(1.f/96000.f);','float phase=s->phase+(float)((double)s->rate*(s->phase<0.f?-.048:.048)/96000.); phase+=(float)(2.*((double)s->rate*hz)/96000.);')
if m in ('delay','phase-delay','all'):
 rep('const float center=mode==3u?314.799988f:142.f;\n    s->center+=(center-s->center)*.0000304996283f;\n    float displacement=mode==3u?28.8f:384.f;','const float center=mode==3u?.01921386644244194f:.0086669921875f;\n    float v=fmaf(center-s->center,3.049962833756581e-05f,s->center);\n    s->center=v==s->center?center:v;\n    const double displacement=mode==3u?28.799999999999997:384.;')
 rep('float ph=phase+((!ch && mode!=3u)?1.f:0.f);\n        if(ph>1.f)ph-=2.f;\n        float d=4.f+s->center+s->depth*displacement*.967567563f*fm_fabsf(ph);','double ph=(double)phase+((!ch && mode!=3u)?1.:0.);\n        if(ph>1.)ph-=2.;\n        float d=(float)(4.+fma(displacement*(double)s->depth,fabs(ph)*.967567563,(double)(s->center*16384.f)));')
if m in ('post','all'):
 rep('s->post0[ch]+=.533294672f*p;', 's->post0[ch]+=.2516540875455158f*p;')
 rep('s->post1[ch]+=.533294672f*s->post0[ch];','s->post1[ch]+=.2516540875455158f*s->post0[ch];')
Path(o).write_text(s)
