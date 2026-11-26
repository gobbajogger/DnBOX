from pathlib import Path
import sys
i,o=sys.argv[1:];s=Path(i).read_text()
def change(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
s=s.replace('.533294672f*p','.2516540875455158f*p').replace('.533294672f*s->post0[ch]','.2516540875455158f*s->post0[ch]')
change('    return (a+(b-a)*(delay-(float)d))*(1.f/4096.f);','''    if(opt&2) {
        a=fr[ch][(s->write-d)&1023u]; b=d<s->filled?fr[ch][(s->write-d-1u)&1023u]:0.f;
        return a+(b-a)*(delay-(float)d);
    }
    return (a+(b-a)*(delay-(float)d))*(1.f/4096.f);''')
change('        c->mode_history[ch][s->write]=j60_quantize(h);','''        tap[ch][at]=h;
        if(opt&1)h=(float)auth[ch][at];
        if(opt&2)fr[ch][s->write]=h;
        else c->mode_history[ch][s->write]=j60_quantize(h);''')
Path(o).write_text(s)
