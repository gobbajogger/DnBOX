"""Make explicitly diagnostic whole-owner schedule from exact paired warm donor."""
import hashlib,sys,difflib
from pathlib import Path
f=Path(sys.argv[1]); orig=f.read_text(); t=orig
assert hashlib.sha256(f.read_bytes()).hexdigest()=="92a7d2f68d027c40306a27bf954ea7f5a736e14cc8d169a97ef9b30e56cb4f54"
def rep(a,b):
 global t
 assert t.count(a)==1,repr(a)
 t=t.replace(a,b)
def sect(a,z):
 i=t.index(a,t.index("static int run_job(int j, int core)"));return t[i:t.index(z,i)]
send=sect("        PROF_T(pb);","        w0 = plat_ticks24();\n        if (stage_wait((uint32_t)(2 + bv), core))")
body='''/* Whole-owner A/B preparation only; no writes to shared dry/send sum.
 * Bass B retains right-left lane order and no shared Chicago memo crosses cores. */
static void bass_prepare_job(int bv, int core, uint32_t n)
{
    int p=PART_303A+bv;
    float *bb[NBASS];
    uint32_t i;
    bb[bv]=eng_bass_buf[2*bv];bb[1-bv]=eng_bass_buf[2*bv+1];
'''+send+'''}

'''
a='/* one job, render and mix; core 1: not timed by the cycle counter (core 0\'s) */'
rep(a,body+a)
rep(send+'        w0 = plat_ticks24();\n        if (stage_wait((uint32_t)(2 + bv), core))','''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
        /* Only C0 can have prepared these inputs at the admitted call site. */
        if (!core && early_bass_ready)
            tk -= early_bass_ticks[bv];
        else
#endif
            bass_prepare_job(bv,core,n);
        int status = bass_render_ok[bv];
        w0 = plat_ticks24();
        if (stage_wait((uint32_t)(2 + bv), core))''')
# Successful render is encoded as a boolean. Scope exact symbol, not all status comparisons.
rep('''        wt = plat_ticks24() - w0;
        if (status == ORIGINAL_RUNTIME_OK) {''','''        wt = plat_ticks24() - w0;
        if (status) {''')
rep('static uint8_t eng_job_solo;', '''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
/* Only C0 writes and reads these; never a second copy of live DSP state. */
static uint32_t early_bass_ticks[NBASS];
static uint8_t early_bass_ready;
#endif
static uint8_t eng_job_solo;''')
rep('    m = plan_pick(n);\n    eng_c1_mask = (uint8_t)m;', '''    m = plan_pick(n);
#if X0X_DIAG_OWNER909_EARLY_CHICAGO
    early_bass_ready=0;
    if (drum909_active(&d909) &&
        original_bass[1].kind == ORIGINAL_SOUND_CHICAGO)
        m=1u << JB_909;   /* complete kit, not an individual sampler */
#endif
    eng_c1_mask = (uint8_t)m;''')
rep('''#if X0X_DEBUG
    eng_dbg_nsub++;
''','''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
    /* After actual worker admission only. No other C1 bass/BREAK publisher.
     * Prefix uses 808 buses; A/B workspaces are released before C0 enters
     * the 808 job and any borrowed metal window. Commit ordering unchanged. */
    if (eng_c1_mask == (1u << JB_909) &&
        original_bass[1].kind == ORIGINAL_SOUND_CHICAGO) {
        for (int bv=0;bv<NBASS;bv++) {
            uint32_t tick=plat_ticks24();
            bass_prepare_job(bv,0,n);
            early_bass_ticks[bv]=plat_ticks24()-tick;
        }
        early_bass_ready=1;
    }
#endif
#if X0X_DEBUG
    eng_dbg_nsub++;
''')
# Never replay a claimed mutable owner after CPU1 halt. Diagnostic trap is
# deliberately non-resumable: no partially advanced schedules can continue.
rep('static uint8_t early_bass_ready;', '''static uint8_t early_bass_ready;
static volatile uint32_t early_909_claim __attribute__((aligned(4)));
static volatile uint32_t early_909_partial_fault __attribute__((aligned(4)));''')
rep('#define eng_sync() plat_cpu2_sync()', '''#define eng_sync() plat_cpu2_sync()
#if X0X_DIAG_OWNER909_EARLY_CHICAGO
static void early_909_check_halted(void)
{
    eng_sync();
    if (eng_c1_mask == (1u << JB_909) &&
        ((early_909_claim == 1 && !eng_808_body) ||
         (early_909_claim == 2 && eng_stage < 1))) {
        early_909_partial_fault=early_909_claim;
        eng_sync();
        __builtin_trap(); /* never reach local replay with partially written state */
    }
}
#endif''')
rep('''            if (!eng_808_body)
                d808_body_run();''','''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
            early_909_check_halted();
#endif
            if (!eng_808_body)
                d808_body_run();''')
rep('''    if (eng_808_split && !eng_808_body)
        d808_body_run();''','''    if (eng_808_split && !eng_808_body) {
#if X0X_DIAG_OWNER909_EARLY_CHICAGO
        if (eng_c1_mask==(1u<<JB_909)) {
            early_909_claim=1;
            eng_sync(); /* claim visible before any prefix state write */
        }
#endif
        d808_body_run();
    }''')
rep('''        if (!run_job(j, 1))''','''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
        if (j==JB_909 && eng_c1_mask==(1u<<JB_909)) {
            early_909_claim=2;
            eng_sync(); /* claim visible before any kit or accumulation write */
        }
#endif
        if (!run_job(j, 1))''')
rep('''    for (j = eng_c1_next; j < JB_N && eng_stage < s; j++)''','''#if X0X_DIAG_OWNER909_EARLY_CHICAGO
    early_909_check_halted();
    /* A halt can retire the job after stage 1 published but before next
     * advanced. A published 909 must never execute again either. */
    if (eng_c1_mask==(1u<<JB_909) && eng_stage>=1)
        eng_c1_next=JB_N;
#endif
    for (j = eng_c1_next; j < JB_N && eng_stage < s; j++)''')
rep('''    early_bass_ready=0;''','''    early_bass_ready=0;
    early_909_claim=0; /* C1 has joined the previous stretch */''')
# Restore the original render branch byte for byte in the ordinary preprocessed
# profile. With the diagnostic compiled out, no factoring alters layout.
a=t.index('/* Whole-owner A/B preparation')
z=t.index('/* one job, render and mix',a)
t=t[:a]+'#if X0X_DIAG_OWNER909_EARLY_CHICAGO\n'+t[a:z]+'#endif\n'+t[z:]
start=t.index('#if X0X_DIAG_OWNER909_EARLY_CHICAGO\n        /* Only C0')
stop=t.index('        w0 = plat_ticks24();\n        if (stage_wait((uint32_t)(2 + bv), core))',start)
new=t[start:stop].splitlines(True)
assert new[0].startswith('#if ') and sum(l.startswith('#endif') for l in new)==1
new=[l for l in new if not l.startswith('#endif')]
t=t[:start]+''.join(new)+'#else\n'+send+'#endif\n'+t[stop:]
rep('''        wt = plat_ticks24() - w0;
        if (status) {''','''        wt = plat_ticks24() - w0;
#if X0X_DIAG_OWNER909_EARLY_CHICAGO
        if (status) {
#else
        if (status == ORIGINAL_RUNTIME_OK) {
#endif''')
name='firmware/src/app/engine.c'
sys.stdout.writelines(difflib.unified_diff(orig.splitlines(True),t.splitlines(True),'a/'+name,'b/'+name))
