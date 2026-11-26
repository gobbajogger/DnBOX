from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/app/engine.c'
t=p.read_text()
a='static uint32_t lab_masks,lab_spans;'
b='''static uint32_t lab_masks,lab_spans,lab_zero,lab_slot;
static uint16_t lab_line[6][12] X0X_POOL;
volatile uint16_t lab_full[6][12] X0X_POOL;
static inline void lab_mark(unsigned field) {
    if (lab_slot<6)
        lab_line[lab_slot][field]=(uint16_t)(1u+((plat_ticks24()-lab_zero)>>3));
}'''
assert t.count(a)==1;t=t.replace(a,b)
a='        lab_morph_trace[15]=lab_wait[0];lab_morph_trace[16]=lab_wait[1];'
b=a+'''
        for(i=0;i<72;i++)lab_full[i/12][i%12]=lab_line[i/12][i%12];'''
assert t.count(a)==1;t=t.replace(a,b)
a='    lab_wait[0]=lab_wait[1]=lab_masks=lab_spans=0;'
t=t.replace(a,a+'\n    __builtin_memset(lab_line,0,sizeof lab_line);')
a='    int j;\n    (void)arg;\n    if (eng_808_split && !eng_808_body)'
assert t.count(a)==1;t=t.replace(a,'    int j;\n    (void)arg;\n    lab_mark(2);\n    if (eng_808_split && !eng_808_body)')
a='        d808_body_run();\n    for (j = eng_c1_next;'
assert t.count(a)==1;t=t.replace(a,'        d808_body_run();\n    lab_mark(3);\n    for (j = eng_c1_next;')
a='    eng_c1_next = JB_N;\n    eng_sync();'
assert t.count(a)==1;t=t.replace(a,'    eng_c1_next = JB_N;\n    lab_mark(5);\n    eng_sync();')
a='        eng_stage = 1;\n    } else if (j == JB_808)'
assert t.count(a)==1;t=t.replace(a,'        eng_stage = 1;\n        if (!core) lab_mark(4);\n    } else if (j == JB_808)')
a='    int j;\n    if (solo & (0x7FFu << 5))'
assert t.count(a)==1;t=t.replace(a,'    int j;\n    lab_slot=lab_spans;\n    if (!lab_slot)lab_zero=plat_ticks24();\n    lab_mark(0);\n    if (solo & (0x7FFu << 5))')
a='    if (m && !plat_cpu2_run(core2_jobs, 0u))'
assert t.count(a)==1;t=t.replace(a,'    lab_mark(1);\n'+a)
a='    if (eng_c1_mask) {\n        PROF_T(pt);'
assert t.count(a)==1;t=t.replace(a,'    lab_mark(6);\n'+a)
a='    lab_masks|=1u<<eng_c1_mask;'
assert t.count(a)==1;t=t.replace(a,'    lab_mark(7);\n'+a)
a='        if (bass_render_ok[j]) original_rec_recover_off(j);'
assert t.count(a)==1;t=t.replace(a,a+'\n    lab_mark(8);')
a='    if (eng_do_master && n >= ENG_TAIL_CS) {'
assert t.count(a)==1;t=t.replace(a,'    lab_mark(9);\n'+a)
a='        eng_fx_done = off;\n        eng_sync();'
assert t.count(a)==1;t=t.replace(a,a+'\n        if(off==n)lab_mark(10);')
a='            eng_master_done = 1;\n            return;'
assert t.count(a)==1;t=t.replace(a,'            eng_master_done = 1;\n            lab_mark(11);\n            return;')
a='}\n\n#if X0X_DIAG_STATE_TRACE\n__attribute__((noinline, minsize)) void engine_diag_half'
assert t.count(a)==1;t=t.replace(a,'    lab_mark(11);\n'+a)
p.write_text(t)
