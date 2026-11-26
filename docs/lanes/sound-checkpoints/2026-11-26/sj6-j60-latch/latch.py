from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text()
for a,b in [('s->mix+=(s->target[2]-s->mix)*ramp;', 'const float previous_mix=s->mix;\n    s->mix+=(s->target[2]-s->mix)*ramp;'),
 ('s->post0[ch]+=','s->wet[ch]=1.17f*previous_mix*s->post1[ch];\n        s->post0[ch]+='),
 ('        s->wet[ch]=s->post1[ch];\n',''),
 ('+1.17f*s->mix*s->wet[0]','+s->wet[0]'),('+1.17f*s->mix*s->wet[1]','+s->wet[1]')]:
 assert s.count(a)==1
 s=s.replace(a,b)
p.write_text(s)
