from pathlib import Path
import math,sys
dest=Path(sys.argv[1])
def make(m,p,f):
 j=range(1-m//2,1+m//2); weights=[]
 for n in j:
    t=p-n; z=math.pi*f*t
    h=(f*math.sin(z)/z if z else f)
    w=.42+.5*math.cos(math.pi*t/(m/2))+.08*math.cos(2*math.pi*t/(m/2)) if abs(t)<m/2 else 0
    weights.append(h*w)
 q=sum(weights);return [x/q for x in weights]
r='#include <assert.h>\n'
for m in (4,8,16):
 phases=256 if m==64 else 8
 r+=f'static const float m{m}[2][{phases+1}][{m}]={{'
 for direction,f in enumerate((1,147/320)):
   r+='{'+','.join('{'+','.join(float(x).hex()+'f' for x in make(m,p/phases,f))+'}' for p in range(phases+1))+'},'
 r+='};\n'
 r+=f'''void tap{m}(const float*l,const float*r,float phase,int d,float*out){{
    unsigned k=(unsigned)(phase*{phases}u);
    assert(k<{phases}u);
    float w=phase*{phases}u-k,x=0.f,y=0.f;
    for(unsigned i=0;i<{m};i++){{
       float a=m{m}[d][k][i],b=m{m}[d][k+1][i];
       a+=(b-a)*w;x+=l[i]*a;y+=r[i]*a;
    }}
    out[0]=x;out[1]=y;
 }}\n'''
dest.write_text(r)
