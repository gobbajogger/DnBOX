from pathlib import Path
import sys
p,o=map(Path,sys.argv[1:])
s=p.read_text().replace('#include <stdio.h>','#include <stdio.h>\n#include <stdlib.h>')
a='  prepare(bpm);';assert s.count(a)==1
s=s.replace(a,a+'''
  const char *seed=getenv("SJ6_VINTAGE_FIXTURE_SEED");
  if(seed)part.dsp.juno.synth.core.sj6_vintage_random=(uint32_t)strtoul(seed,0,0);
  if(getenv("SJ6_VINTAGE_FIXTURE_ZERO"))dnbx_set_sj6_vintage(&part.dsp.juno.synth.core,0,0);
''')
a='    assert(original_runtime_render(&part,&work,l,r,frames)==ORIGINAL_RUNTIME_OK);'
assert s.count(a)==1
s=s.replace(a,a+r'''
    if(seed && (total==16384u||total==38434u||total==49459u||total==82534u)) {
      dnbx_voice *v=&part.dsp.juno.synth.core.voice[0];
      fprintf(stderr,"OWNER %u %u %a %a %u %u %d %d %a %a\n",total,v->gate,
        v->layer[0].amp.level,v->layer[1].amp.level,v->layer[0].amp.stage,v->layer[1].amp.stage,
        v->sj6_vintage_q8[0],v->sj6_vintage_q8[1],v->layer[0].increment,v->layer[1].increment);
    }
''')
o.write_text(s)
