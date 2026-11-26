import sys
from pathlib import Path
p,o=map(Path,sys.argv[1:]);s=p.read_text()
a='  prepare(bpm);'
assert s.count(a)==1
s=s.replace(a,a+r'''
  fprintf(stderr,"projection %a %a %u %u\n", part.dsp.juno.synth.core.layer[0].params.waveform,
    part.dsp.juno.synth.core.layer[1].params.waveform,part.dsp.juno.synth.osc1_level_code,part.dsp.juno.synth.osc2_level_code);
''')
s=s.replace('#include <stdio.h>','#include <stdio.h>\n#include <stdlib.h>')
s=s.replace('static float peak[2][2];','static FILE *stages[2];\nstatic float peak[2][2];')
a='void witness(unsigned stage,const float *l,const float *r,unsigned n){'
assert s.count(a)==1
s=s.replace(a,a+"""
    if(stages[stage])for(unsigned i=0;i<n;i++){
        const float pair[2]={l[i],r[i]};
        assert(fwrite(pair,sizeof(float),2,stages[stage])==2);
    }
""")
s=s.replace('  prepare(bpm);',"""
  const char *tap=getenv("SJ6_STAGE");
  assert(tap);
  char path[1024];
  for(unsigned d=0;d<2;d++){
     assert(snprintf(path,sizeof path,"%s-stage%u.f32",tap,d)>0);
     stages[d]=fopen(path,"wb"); assert(stages[d]);
  }
  prepare(bpm);
""")
o.write_text(s)
