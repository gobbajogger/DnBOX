import sys,hashlib,subprocess,difflib,os
from pathlib import Path
root=Path(sys.argv[1]); base=root/'firmware/src'
want={'app/engine.c':'92a7d2f68d027c40306a27bf954ea7f5a736e14cc8d169a97ef9b30e56cb4f54',
'dsp/drum909.c':'69e6f18b56ef6c710ed018dd352d5c79eac5faabc1fe653b40606287c8a6c5d7','app/original_runtime.h':'5c8d544904b3dc05d60ef395cb41e11bd8e6d36e25facd22e2c71a4aa218b4ce'}
def source(f):
 raw=(base/f).read_bytes()
 if f in want:assert hashlib.sha256(raw).hexdigest()==want[f],f
 return raw.decode()
def transform(f,changes):
 before=source(f); after=before
 for a,b in changes:
  assert after.count(a)==1,(f,a,after.count(a))
  after=after.replace(a,b)
 return after
def diff(f,after):
 before=source(f);name='firmware/src/'+f
 sys.stdout.writelines(difflib.unified_diff(before.splitlines(True),after.splitlines(True),'a/'+name,'b/'+name))
f='dsp/drum909.c'
s=subprocess.check_output([sys.executable,str(Path(__file__).resolve().parents[1]/'owner909-terminal-group/group.py'),str(base/f)],text=True)
# Convert tested host-only atomics to explicit target publication barriers.
s=s.replace('#include <stdatomic.h>\ntypedef struct {d9_smp_t voices[4];float mono[4][256];_Atomic unsigned ready;} terminal_future;\nstatic unsigned terminal_token(unsigned stamp,unsigned n)\n{\n    return ((stamp&0x7fffffu)<<9)|((n-1u)<<1);\n}\nstatic void terminal_begin(terminal_future*f,const drum909_t*d,unsigned token)', '''extern void plat_cpu2_sync(void);
void drum909_terminal_begin(terminal_future*f,const drum909_t*d,unsigned token)''')
s=s.replace('atomic_store_explicit(&f->ready,token,memory_order_relaxed);','f->ready=token; /* published by core dispatch after snapshot */')
s=s.replace('static void terminal_create(terminal_future*f,int n,unsigned token)','void drum909_terminal_compute(terminal_future*f,int n,unsigned token)')
s=s.replace('atomic_store_explicit(&f->ready,token|1u,memory_order_release);','plat_cpu2_sync();\n    f->ready=token|1u;\n    plat_cpu2_sync();')
s=s.replace('    const int prepared = future && atomic_load_explicit(&future->ready,memory_order_acquire)==(expected|1u);','''    const int prepared = future && future->ready==(expected|1u);
    if(prepared) plat_cpu2_sync(); /* acquire before private state/stem dereference */''')
a='void drum909_render_st_cleared('
s=s.replace(a,'''void drum909_render_st_terminal(drum909_t*d,float*l,float*r,float*v,float*y,int n,uint16_t tracks,
                                terminal_future*f,unsigned token)
{
    d9_render_st_masked(d,l,r,v,y,n,tracks,1,f,token);
}
'''+a,1)
# Keep OFF unchanged: insert only conditional helper/parameter/branch seams.
s=s.replace('/* Quarantined fixture only. Actual firmware may not opt in. */','#if X0X_DIAG_909_TERMINAL_GROUP\n/* Never-install diagnostic. */')
s=s.replace('static void d9_render(drum909_t', '#endif\nstatic void d9_render(drum909_t',1)
s=s.replace(',terminal_future *future,unsigned expected)', '\n#if X0X_DIAG_909_TERMINAL_GROUP\n                      ,terminal_future *future,unsigned expected\n#endif\n                     )')
s=s.replace('    const int prepared =',' #if X0X_DIAG_909_TERMINAL_GROUP\n    const int prepared =').replace('if(prepared) plat_cpu2_sync(); /* acquire before private state/stem dereference */','if(prepared) plat_cpu2_sync(); /* acquire before private state/stem dereference */\n#endif')
s=s.replace('            if(prepared && s->playing) {','#if X0X_DIAG_909_TERMINAL_GROUP\n            if(prepared && s->playing) {').replace('            } else d9_render_smp(s, &bus, n);','            } else\n#endif\n                d9_render_smp(s, &bus, n);')
for a,b in [('0x7FFu,0,0','0x7FFu'),('audible_tracks,future,expected','audible_tracks'),('audible_tracks,1,0,0','audible_tracks,1'),('audible_tracks,0,0,0','audible_tracks,0')]:
 assert s.count(a)==1
 s=s.replace(a,b+'\n#if X0X_DIAG_909_TERMINAL_GROUP\n                '+a[len(b):]+'\n#endif\n                ')
start=s.index('void drum909_render_st_terminal(');end=s.index('void drum909_render_st_cleared(',start)
s=s[:start]+'#if X0X_DIAG_909_TERMINAL_GROUP\n'+s[start:end]+'#endif\n'+s[end:]
if os.environ.get('TERMINAL_COMPACT')=='1':
 a='    for(unsigned i=0;i<4;i++){\n        d9_bus_t b={.dry=0,.y=f->mono[i]};'
 assert s.count(a)==1
 s=s.replace(a,'    d9_bus_t b={.dry=0};\n    for(unsigned i=0;i<4;i++){\n        b.y=f->mono[i];')
diff(f,s)
f='dsp/drum909.h'
s=transform(f,[('typedef struct drum909 drum909_t;', '''typedef struct drum909 drum909_t;
#if X0X_DIAG_909_TERMINAL_GROUP
typedef struct {
    d9_smp_t voices[4];
    float mono[4][256];
    volatile uint32_t ready;
} terminal_future;
void drum909_terminal_begin(terminal_future *,const drum909_t *,unsigned);
void drum909_terminal_compute(terminal_future *,int,unsigned);
void drum909_render_st_terminal(drum909_t *,float *,float *,float *,float *,int,uint16_t,terminal_future *,unsigned);
#endif''')])
diff(f,s)
f='app/original_runtime.h'
s=transform(f,[('    float metal[4][256];', '''#if X0X_DIAG_909_TERMINAL_GROUP
    terminal_future terminal;
#endif
    float metal[4][256];''')])
diff(f,s)
f='app/engine.c'
s=transform(f,[
('static uint8_t eng_808_split;', '''#if X0X_DIAG_909_TERMINAL_GROUP
_Static_assert(sizeof(terminal_future)<=5120
  && __alignof__(terminal_future)>=4
  && __builtin_offsetof(terminal_future,ready)%4==0,"909 group must fit A workspace and align readiness");
static uint32_t eng_terminal_token;           /* C0 only before dispatch/after halt */
static volatile uint32_t eng_terminal_claim;  /* sole C1 claimant after dispatch */
static volatile uint32_t eng_terminal_fault;  /* C0 failure reason for diagnostic */
#endif
static uint8_t eng_808_split;'''),
('#define eng_sync() plat_cpu2_sync()', '''#define eng_sync() plat_cpu2_sync()
#if X0X_DIAG_909_TERMINAL_GROUP
static void terminal_check_halted(void)
{
    if (!eng_terminal_token)
        return;
    eng_sync(); /* call only after worker was held or joined */
    uint32_t claim=eng_terminal_claim;
    if ((claim==1u && !eng_808_body) || (claim==2u && eng_stage<3u)) {
        eng_terminal_fault=claim;
        eng_sync();
        __builtin_trap(); /* never replay partially mutable original prefix/A */
    }
}
#endif'''),
('''            if (!eng_808_body)
                d808_body_run();''','''#if X0X_DIAG_909_TERMINAL_GROUP
            terminal_check_halted();
#endif
            if (!eng_808_body)
                d808_body_run();'''),
('''            drum909_render_st_cleared(&d909,dry_l,dry_r,rev,dly,(int)n,
                        smc_audible_drums(solo,PART_909));''','''#if X0X_DIAG_909_TERMINAL_GROUP
            if (eng_terminal_token)
                drum909_render_st_terminal(&d909,dry_l,dry_r,rev,dly,(int)n,
                    smc_audible_drums(solo,PART_909),&original_workspace.terminal,eng_terminal_token);
            else
#endif
                drum909_render_st_cleared(&d909,dry_l,dry_r,rev,dly,(int)n,
                        smc_audible_drums(solo,PART_909));'''),
('''    if (eng_808_split && !eng_808_body)
        d808_body_run();''','''    if (eng_808_split && !eng_808_body) {
#if X0X_DIAG_909_TERMINAL_GROUP
        if(eng_terminal_token) {
            eng_terminal_claim=1;
            eng_sync(); /* claim before first mutable 808 prefix write */
        }
#endif
        d808_body_run();
    }
#if X0X_DIAG_909_TERMINAL_GROUP
    if(eng_terminal_token) {
        const unsigned n=eng_job_arg & 255u;
        drum909_terminal_compute(&original_workspace.terminal,(int)(n?n:256u),eng_terminal_token);
        /* A is active: private producer is already joined on this worker,
         * and A cannot reuse the same workspace until C0 has consumed OR
         * taken the nonwaiting serial fallback and published stage 1. */
        if(stage_wait(1,1)) return;
        eng_sync(); /* acquire even if stage_wait returned via its fast path */
    }
#endif'''),
('''        if (!run_job(j, 1))
            return;''','''#if X0X_DIAG_909_TERMINAL_GROUP
        if(eng_terminal_token && j==JB_A) {
            eng_terminal_claim=2;
            eng_sync(); /* claim before actual A or its live output side effects */
        }
#endif
        if (!run_job(j, 1))
            return;'''),
('''    eng_sync();
    for (j = eng_c1_next; j < JB_N && eng_stage < s; j++)''','''    eng_sync();
#if X0X_DIAG_909_TERMINAL_GROUP
    terminal_check_halted();
    if(eng_terminal_token && eng_stage>=3u)
        eng_c1_next=JB_N; /* completed A cannot replay, even with old next index */
#endif
    for (j = eng_c1_next; j < JB_N && eng_stage < s; j++)'''),
('''    eng_808_body = (uint8_t)!eng_808_split;
    eng_sync();''','''    eng_808_body = (uint8_t)!eng_808_split;
#if X0X_DIAG_909_TERMINAL_GROUP
    eng_terminal_token=0;
    eng_terminal_claim=0; /* previous worker already held/joined */
#ifndef X0X_HOST
    if (m==(1u<<JB_A) && n && n<=256 && drum909_active(&d909)) {
        eng_terminal_token=0x80000000u | ((d8_frame&0x3fffffu)<<9) | ((n-1u)<<1);
        drum909_terminal_begin(&original_workspace.terminal,&d909,eng_terminal_token);
    }
#endif
#endif
    eng_sync();'''),
('''        eng_808_body = 1;
    }
#if X0X_DEBUG''','''        eng_808_body = 1;
#if X0X_DIAG_909_TERMINAL_GROUP
        eng_terminal_token=0;
#endif
    }
#if X0X_DEBUG'''),
])
if os.environ.get('TERMINAL_COMPACT')=='1':
 a='static void terminal_check_halted(void)'
 assert s.count(a)==1
 s=s.replace(a,'__attribute__((noinline))\n'+a)
diff(f,s)
