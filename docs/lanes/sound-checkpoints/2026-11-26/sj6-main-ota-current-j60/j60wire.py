from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='                juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);'
assert s.count(a)==1
p.write_text(s.replace(a,'''            {
                extern int submit_j60(float*,float*,unsigned,unsigned);
                if(!submit_j60(left,right,frames,p->dsp.juno.chorus.factory_mode))
                    juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);
            }'''))
