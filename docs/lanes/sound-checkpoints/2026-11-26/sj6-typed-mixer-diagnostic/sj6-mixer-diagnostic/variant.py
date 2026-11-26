from pathlib import Path
import sys
p,o=map(Path,sys.argv[1:3]); a=sys.argv[3]; s=p.read_text()
x='float x=mix[l]*(p->gain/6.0f);'
pan='left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);'
assert s.count(x)==s.count(pan)==1
if a!='base':
    s=s.replace(x,'float x=mix[l]*(s->superj6_amp_range?p->gain:p->gain/6.0f);')
if a=='center':
    s=s.replace(pan,'if(s->superj6_amp_range && p->pan==0.f){left[k]+=x;right[k]+=x;}else{'+pan+'}')
o.write_text(s)
