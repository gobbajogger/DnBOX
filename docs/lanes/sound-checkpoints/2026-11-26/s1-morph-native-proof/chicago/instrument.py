from pathlib import Path
import sys
d=Path(sys.argv[1])/'tree/firmware/src'
p=d/'dsp/original_chicago_backend.c';t=p.read_text()
a='static int end_admitted('
k=t.index(a); declaration='''extern uint32_t plat_ticks24(void);
extern uint32_t lab_chicago_work[4];
'''
t=declaration+t if False else t[:k]+t[k:]
# In rendering, record only admission, dry scheduler+voice, and wet output/copy.
# Admission excludes the independent runtime prepare that may already have run.
q=t.index('int original_chicago_backend_render(')
a='    int result = begin_admitted(s, w, events, count, frames);'
sz=t.index(a,q);t=t[:sz]+'''    uint32_t start = plat_ticks24();
'''+t[sz:]
a='    if (result != ORIGINAL_CHICAGO_BACKEND_OK) return result;'
i=t.index(a,sz);i+=len(a);t=t[:i]+'''
    lab_chicago_work[0] += plat_ticks24() - start;
    start = plat_ticks24();
'''+t[i:]
q=t.index('int original_chicago_backend_render('); e=t.index('/* Display only',q)
mid=t[q:e];first='return end_admitted(s, left, right, '
def wrap(z,zero):
 a=first+str(zero)+');'
 return z.replace(a,'''lab_chicago_work[1] += plat_ticks24() - start;
    return end_admitted(s, left, right, '''+str(zero)+');')
mid=wrap(wrap(mid,0),1)
t=t[:q]+mid+t[e:]
q=t.index('static int end_admitted(')
t=t[:q]+declaration+t[q:]
q=t.index('static int end_admitted(');a='    if (!output(&s->output'
k=t.index(a,q);t=t[:k]+'    uint32_t wet_start=plat_ticks24();\n'+t[k:]
a='    memcpy(left, w->core_left, sizeof *left * s->block_frames);'
k=t.index(a,q);t=t[:k]+'''    lab_chicago_work[2]+=plat_ticks24()-wet_start;
    wet_start=plat_ticks24();
'''+t[k:]
a='    s->block_open = 0;';k=t.index(a,q);t=t[:k]+'''    lab_chicago_work[3]+=plat_ticks24()-wet_start;
'''+t[k:];p.write_text(t)
p=d/'app/main_fm1.c';t=p.read_text()
a='    x0x_dbg.last_us = us;'
b='''    extern uint32_t lab_chicago_work[4];
    extern volatile uint32_t lab_chicago_worst[7];
    if (us > x0x_dbg.max_us) {
        lab_chicago_worst[0]=x0x_dbg.halves;
        lab_chicago_worst[1]=us;
        lab_chicago_worst[2]=plat_ms();
        for (unsigned i=0;i<4;i++) lab_chicago_worst[i+3]=lab_chicago_work[i];
    }
    for (unsigned i=0;i<4;i++) lab_chicago_work[i]=0;
'''+a
assert t.count(a)==1;t=t.replace(a,b)
idx=t.index('struct x0x_dbg {')
t=t[:idx]+'''uint32_t lab_chicago_work[4] X0X_POOL;
volatile uint32_t lab_chicago_worst[7] X0X_POOL;
'''+t[idx:];p.write_text(t)
