import sys
from pathlib import Path
p=Path(sys.argv[1]);st=p.read_text(); import hashlib
sites={'    if(routed && !s->sj6_pitch_routed && !s->sj6_pitch_depth)':'    assert(!routed);\n',
       '    if(code[4]>60u)return 0;':'    assert(!code[4]);\n',
       '      )envelope_on(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate);':'      )assert(!"PitchEnv on while reserved");\n',
       '      )envelope_off(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate);':'      )assert(!"PitchEnv off while reserved");\n',
       '                float env=envelope_sample(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate,w);':'                assert(!"PitchEnv render while reserved");\n'}
for a,b in sites.items():
 assert st.count(a)==1,a
 if a.startswith('      )envelope_'):st=st.replace(a,b)
 else:st=st.replace(a,b+a)
p.write_text(st)
