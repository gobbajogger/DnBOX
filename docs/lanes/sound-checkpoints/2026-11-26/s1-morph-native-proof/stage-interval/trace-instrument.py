from pathlib import Path
import sys
d=Path(sys.argv[1]); p=d/'tree/firmware/src/app/engine.c'; t=p.read_text()
marker='static uint8_t eng_brk_mark;'
i=t.index(marker); t=t[:i]+'''static uint32_t lab_exec[10] X0X_POOL;
static uint32_t lab_wait[2] X0X_POOL;
static uint32_t lab_masks,lab_spans;
volatile uint32_t lab_morph_trace[17] X0X_POOL;
__attribute__((noinline)) void lab_morph_trace_half(uint32_t half,uint32_t us)
{
    uint32_t i;
    if (half==1) lab_morph_trace[1]=0;
    if (us>lab_morph_trace[1]) {
        lab_morph_trace[0]=half; lab_morph_trace[1]=us;
        lab_morph_trace[2]=plat_ms(); lab_morph_trace[3]=lab_masks;
        lab_morph_trace[4]=lab_spans;
        for(i=0;i<10;i++) lab_morph_trace[5+i]=lab_exec[i];
        lab_morph_trace[15]=lab_wait[0];lab_morph_trace[16]=lab_wait[1];
    }
    for(i=0;i<10;i++) lab_exec[i]=0;
    lab_wait[0]=lab_wait[1]=lab_masks=lab_spans=0;
}
'''+t[i:]
a='''    if (active)
        job_cost(j, plat_ticks24() - tk - wt, n);
    return 1;'''
b='''    if (active) {
        uint32_t execution=plat_ticks24()-tk-wt;
        job_cost(j,execution,n);
        lab_exec[5*core+j]+=execution;
    }
    lab_wait[core] += wt;
    return 1;'''
assert t.count(a)==1;t=t.replace(a,b)
a='''    brk_midi_drain();
    for (j = 0; j < NBASS; j++)'''
b='''    lab_masks|=1u<<eng_c1_mask;
    ++lab_spans;
    brk_midi_drain();
    for (j = 0; j < NBASS; j++)'''
assert t.count(a)==1;t=t.replace(a,b);p.write_text(t)
p=d/'tree/firmware/src/app/main_fm1.c';t=p.read_text()
a='    x0x_dbg.last_us = us;';b='    extern void lab_morph_trace_half(uint32_t,uint32_t);\n    lab_morph_trace_half(x0x_dbg.halves,us);\n'+a
assert t.count(a)==1;t=t.replace(a,b);p.write_text(t)
