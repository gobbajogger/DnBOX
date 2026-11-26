import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
a='const float *q,float g,float high,float blend,float outgain)'; assert s.count(a)==1
s=s.replace(a,'const float *q,float g,float high,float blend,float outgain,float gz,float gz_inverse)')
a='    const float gz=g/(1.f+g);\n    const float gz_inverse=1.f-gz;\n';assert s.count(a)==1;s=s.replace(a,'')
a='        a=clock_2x';
if a not in s:a='    a=clock_2x'
assert s.count(a)==1
s=s.replace(a,'    const float g=params[0], gz=g/(1.f+g), gz_inverse=1.f-gz;\n'+a)
a='params[1],params[2],params[3]);';assert s.count(a)==2;s=s.replace(a,'params[1],params[2],params[3],gz,gz_inverse);')
Path(sys.argv[2]).write_text(s)
