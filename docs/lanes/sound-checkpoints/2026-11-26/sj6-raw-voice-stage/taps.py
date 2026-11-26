import sys
from pathlib import Path
p=Path(sys.argv[1]);s=p.read_text()
a='if(s->sj6_ota)mix[l]+=velocity*amp*sj6_ota_filter(lv,x,a,p->resonance);'
assert s.count(a)==1
s=s.replace(a,'''if(s->sj6_ota){
        float filt=sj6_ota_filter(lv,x,a,p->resonance);
        float post=velocity*amp*filt;
        extern void audit_add(unsigned,float,float);
        audit_add(l,x,post);mix[l]+=post;
      }''')
a='        left[k]=0; right[k]=0;';assert s.count(a)==1
s=s.replace(a,'''        {extern void audit_step(void);audit_step();}
'''+a)
p.write_text(s)
