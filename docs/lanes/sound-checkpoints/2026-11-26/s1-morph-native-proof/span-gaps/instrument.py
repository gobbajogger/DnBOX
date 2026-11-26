from pathlib import Path
import sys
d=Path(sys.argv[1])/'tree/firmware/src/app'
p=d/'engine.c';t=p.read_text()
a='static uint8_t eng_brk_mark;'
code='''static uint32_t gap_ticks[8] X0X_POOL;
volatile uint32_t gap_worst[11] X0X_POOL;
static uint32_t gap_prev;
#define GAP(i) do {uint32_t now=plat_ticks24();gap_ticks[i]+=now-gap_prev;gap_prev=now;}while(0)
void gap_half(uint32_t half,uint32_t us)
{
    if(half==1)gap_worst[1]=0;
    if(us>gap_worst[1]){
        gap_worst[0]=half;gap_worst[1]=us;gap_worst[2]=plat_ms();
        for(unsigned i=0;i<8;i++)gap_worst[3+i]=gap_ticks[i];
    }
    __builtin_memset(gap_ticks,0,sizeof gap_ticks);
}
'''
assert t.count(a)==1;t=t.replace(a,code+a)
beg=t.index('void engine_render(int32_t *out_lr')
s=t[beg:];t=t[:beg]
for a,b in [
('    PROF_FRAMES(n);','    gap_prev=plat_ticks24();\n    PROF_FRAMES(n);'),
('    original_prepare_all(); /* precedes every serial frame-zero edit and key */','    GAP(0);\n    original_prepare_all(); /* precedes every serial frame-zero edit and key */\n    GAP(1);'),
('    while (done < n) {\n        uint32_t k = seq_until_event','    GAP(2);\n    while (done < n) {\n        uint32_t k = seq_until_event'),
('        render_sub(out_l + done, out_r + done, k, 0);','        GAP(3);\n        render_sub(out_l + done, out_r + done, k, 0);\n        gap_prev=plat_ticks24();'),
('            original_prepare_all(); /* next actual sequencer stretch, before its events */','            original_prepare_all(); /* next actual sequencer stretch, before its events */\n            GAP(4);'),
('        done += k;\n    }','        done += k;\n        GAP(5);\n    }'),
('    {   /* globals in registers: the stores below cannot reload them per sample */','    GAP(6);\n    {   /* globals in registers: the stores below cannot reload them per sample */'),
('        part_peak[p] = 0.0f;\n    }\n}', '        part_peak[p] = 0.0f;\n    }\n    GAP(7);\n}')]:
 assert s.count(a)==1,(a,s.count(a));s=s.replace(a,b)
p.write_text(t+s)
p=d/'main_fm1.c';t=p.read_text();a='    x0x_dbg.last_us = us;';assert t.count(a)==1;p.write_text(t.replace(a,'    extern void gap_half(uint32_t,uint32_t);\n    gap_half(x0x_dbg.halves,us);\n'+a))
