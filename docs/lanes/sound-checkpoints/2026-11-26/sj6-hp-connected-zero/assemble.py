from pathlib import Path
import sys,difflib,hashlib
base=Path(sys.argv[1]); folder=Path(__file__).parent
patch=[]
for name in ('engine.c','original_runtime.c','original_runtime.h'):
 p=base/'firmware/src/app'/name
 old=p.read_text();s=old
 def rep(a,b,cnt=1):
  global s
  assert s.count(a)==cnt,(name,a,s.count(a))
  s=s.replace(a,b)
 if name=='engine.c':
  rep('static original_runtime_part original_bass[NBASS] POOL;', '''static original_runtime_part original_bass[NBASS] POOL;
#if X0X_SJ6_EXACT_ZERO_LEAF
_Static_assert(NBASS==2, "one independent HP/Crusher state per physical bass owner");
_Static_assert(sizeof(sj6_zero_fx_channel[2])==48, "measured exact leaf state");
/* Plain zero-initialized BSS, never POOL, preview scratch or borrowed FX. */
static sj6_zero_fx_channel original_sj6_zero_leaf[NBASS][2];
static uint8_t original_sj6_zero_profile[NBASS];
static unsigned sj6_zero_owner(const original_runtime_part *owner)
{
    if(owner==&original_bass[0])return 0;
    if(owner==&original_bass[1])return 1;
    return NBASS;
}
sj6_zero_fx_channel *engine_sj6_zero_leaf(const original_runtime_part *owner)
{
    const unsigned v=sj6_zero_owner(owner);
    return v<NBASS && original_sj6_zero_profile[v] ? original_sj6_zero_leaf[v] : NULL;
}
void engine_sj6_zero_clear(const original_runtime_part *owner,unsigned revoke_profile)
{
    const unsigned v=sj6_zero_owner(owner);
    if(v>=NBASS)return;
    memset(original_sj6_zero_leaf[v],0,sizeof original_sj6_zero_leaf[v]);
    if(revoke_profile)original_sj6_zero_profile[v]=0;
}
int engine_sj6_zero_pin(const original_runtime_part *owner,const float physical[8])
{
    const unsigned v=sj6_zero_owner(owner);
    if(v>=NBASS)return 0;
    /* Trusted source readback only: [HP Hz,Q dB, Distortion #3,#4,#5,#15, authored Lo-Fi #109, connected-not-bypassed proof].
     * The recovered two-byte project carrier cannot authenticate these. */
    const unsigned exact=physical && physical[0]==10.f && physical[1]==0.f &&
        physical[2]==.44387006759643555f && physical[3]==0.f &&
        physical[4]==0.f && physical[5]==0.f && physical[6]==0.f && physical[7]==1.f;
    original_sj6_zero_profile[v]=(uint8_t)exact;
    return (int)exact;
}
#endif''')
 elif name=='original_runtime.h':
  rep('} original_runtime_part;', '''} original_runtime_part;
#if X0X_SJ6_EXACT_ZERO_LEAF
#include "../dsp/sj6_hp_zero_leaf.h"
sj6_zero_fx_channel *engine_sj6_zero_leaf(const original_runtime_part *);
void engine_sj6_zero_clear(const original_runtime_part *,unsigned);
int engine_sj6_zero_pin(const original_runtime_part *,const float physical[8]);
#endif''')
 else:
  rep('    memset(p, 0, sizeof *p);', '''#if X0X_SJ6_EXACT_ZERO_LEAF
    engine_sj6_zero_clear(p,1);
#endif
    memset(p, 0, sizeof *p);''')
  rep('    p->status = ORIGINAL_RUNTIME_INVALID;\n    return ORIGINAL_RUNTIME_OK;', '''#if X0X_SJ6_EXACT_ZERO_LEAF
    engine_sj6_zero_clear(p,1);
#endif
    p->status = ORIGINAL_RUNTIME_INVALID;
    return ORIGINAL_RUNTIME_OK;''')
  rep('        juno6_all_off(&p->dsp.juno.synth);', '''        juno6_all_off(&p->dsp.juno.synth);
#if X0X_SJ6_EXACT_ZERO_LEAF
        engine_sj6_zero_clear(p,0);
#endif''')
  rep('static void sj6_run_effects(original_runtime_part *p,float *left,float *right,unsigned frames','''#if X0X_SJ6_EXACT_ZERO_LEAF
#if !X0X_SJ6_NATIVE_ROOM || !X0X_SUPERJ6_BUILTIN_INIT || !X0X_SJ6_AUX_LFO_NATIVE
#error Exact zero leaf needs the typed SuperJ6 FX owner and native room/Crusher controls
#endif
static sj6_zero_fx_channel *sj6_zero_leaf_admit(original_runtime_part *p)
{
    juno6_t *s=&p->dsp.juno.synth;
    /* -1 is the typed HP panel's authored zero, NOT the legacy/bypass 0.
     * The old phase-free HP renderer does nothing at this typed sentinel. */
    if(!s->typed || s->core.sample_rate!=44100.f || s->hpf!=-1.f ||
       p->dsp.juno.crush.active || p->dsp.juno.crush.step!=1.f)return NULL;
    /* Actual current #107=0 sets AU wet=0 and Booster unity; it
     * does not prove a genuine AU bypass. Distinguish it by owner proof.
     * Admission is per span; a modulated nonzero rate returns to legacy. */
    return engine_sj6_zero_leaf(p);
}
static void sj6_exact_hp(sj6_zero_fx_channel *state,float *l,float *r,unsigned n)
{
    for(unsigned i=0;i<n;i++){
        l[i]=sj6_hp10_44100(state,l[i]);
        r[i]=sj6_hp10_44100(state+1,r[i]);
    }
}
static void sj6_exact_zero_crusher(sj6_zero_fx_channel *state,float *l,float *r,unsigned n)
{
    for(unsigned i=0;i<n;i++){
        l[i]=sj6_zero_crusher_44100(state,l[i]);
        r[i]=sj6_zero_crusher_44100(state+1,r[i]);
    }
}
#endif
#if X0X_SJ6_NATIVE_UNISON
static void sj6_unison_span(original_runtime_part *p,float *left,float *right,unsigned frames
#if X0X_SJ6_AUX_LFO_NATIVE
                            ,const float *mod
#endif
                            )
{
#if X0X_SJ6_AUX_LFO_NATIVE
    if(mod && (mod[1]>=0.f || mod[2]>=0.f))
        juno6_unison_process_mod(&p->dsp.juno.synth,left,right,frames,mod[1],mod[2]);
    else
#endif
    juno6_unison_process(&p->dsp.juno.synth,left,right,frames);
}
#endif
static void sj6_run_effects(original_runtime_part *p,float *left,float *right,unsigned frames''')
  marker='''                            )
{
#if (defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)) || X0X_SJ6_ALT3_NATIVE'''
  rep(marker,'''                            )
{
#if X0X_SJ6_EXACT_ZERO_LEAF
    /* Authentic local prefix: Unison -> instrument HP -> premod ->
     * Crusher/booster. Remaining stock downstream stages are unchanged. */
    sj6_zero_fx_channel *leaf=sj6_zero_leaf_admit(p);
    if(leaf) {
#if X0X_SJ6_NATIVE_UNISON
        sj6_unison_span(p,left,right,frames,mod);
#endif
        sj6_exact_hp(leaf,left,right,frames);
    }
#endif
#if (defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)) || X0X_SJ6_ALT3_NATIVE''')
  start='''#endif
#if X0X_SJ6_NATIVE_J60
            if(p->dsp.juno.chorus.j60.selected)'''
  rep(start,'''#endif
#if X0X_SJ6_EXACT_ZERO_LEAF
            if(leaf)sj6_exact_zero_crusher(leaf,left,right,frames);
#endif
#if X0X_SJ6_NATIVE_J60
            if(p->dsp.juno.chorus.j60.selected)''')
  rep('''#if X0X_SJ6_NATIVE_UNISON
            #if X0X_SJ6_AUX_LFO_NATIVE
            if(mod && (mod[1]>=0.f || mod[2]>=0.f))
                juno6_unison_process_mod(&p->dsp.juno.synth,left,right,frames,mod[1],mod[2]);
            else
#endif
            juno6_unison_process(&p->dsp.juno.synth,left,right,frames);
#endif
#if X0X_SJ6_NATIVE_ROOM
            sj6_crush_process(p,left,right,frames);''','''#if X0X_SJ6_NATIVE_UNISON
#if X0X_SJ6_EXACT_ZERO_LEAF
            if(!leaf)
#endif
            sj6_unison_span(p,left,right,frames
#if X0X_SJ6_AUX_LFO_NATIVE
                            ,mod
#endif
                            );
#endif
#if X0X_SJ6_NATIVE_ROOM
#if X0X_SJ6_EXACT_ZERO_LEAF
            if(!leaf)
#endif
            sj6_crush_process(p,left,right,frames);''')
 outpath='firmware/src/app/'+name
 patch+=difflib.unified_diff(old.splitlines(True),s.splitlines(True),'a/'+outpath,'b/'+outpath)
 print(hashlib.sha256(p.read_bytes()).hexdigest(),outpath)
leaf=(folder/'hp_zero_leaf.h').read_text()
patch+=difflib.unified_diff([],leaf.splitlines(True),'/dev/null','b/firmware/src/dsp/sj6_hp_zero_leaf.h')
(folder/'candidate.patch').write_text(''.join(patch))
