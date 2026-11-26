/* Source-verified diagnostic owner. Keep exact callback boundaries,
 * compact pitch/filter/note handling and all control words intact.
 * No alternate identity if previous source AMP is still audible at reattack. */
#include "warm_amp.h"
static sj6_warm_amp src_amp[2];
static unsigned span_words, consumed_words, offset_words, amp_on_count,amp_off_count,amp_words;
static int amp_scope;
static float amp_tape[2][128];
static unsigned char amp_live[128];
static void amp_open(void)
{
    const int ids[2][4]={{36,37,38,39},{119,120,121,122}};
    juno6_t *s=&part.dsp.juno.synth;
    amp_scope=HOST_AMP_EXTERNAL && suffix_scope && s->typed && s->core.sample_rate==44100.f &&
        word[32]==1.f && word[33]==1.f && word[35]==0.f &&
        word[201]==0.f && word[202]==0.f;
    for(unsigned i=0;i<2;i++)for(unsigned k=0;k<4;k++)
        if(!present[ids[i][k]]||word[ids[i][k]]<0.f||word[ids[i][k]]>1.f)amp_scope=0;
    if(!amp_scope)return;
    for(unsigned i=0;i<2;i++)
        sj6_amp_config_panel(&src_amp[i],44100.f,word[ids[i][0]],word[ids[i][1]],word[ids[i][2]],word[ids[i][3]]);
}
static void amp_edge(unsigned was_gate)
{
    if(!amp_scope)return;
    unsigned gate=part.dsp.juno.synth.core.voice[0].gate;
    if(gate==was_gate)return;
    if(gate)amp_on_count++;else amp_off_count++;
    for(unsigned i=0;i<2;i++){
        if(gate){
            /* Pending/releasing AMP owners require their own oscillator
             * phase and filter-lifetime implementation. Fail closed. */
            assert(src_amp[i].stage==0 && src_amp[i].out==0.f);
            sj6_amp_on(&src_amp[i]);
        }else sj6_amp_off(&src_amp[i]);
    }
}
static void amp_span(unsigned n)
{
    if(!amp_scope)return;
    assert(n<=128);
    span_words=n;consumed_words=0;
    for(unsigned at=0;at<n;){
        unsigned m=n-at<16?n-at:16;
        int enabled=src_amp[0].stage||src_amp[1].stage||src_amp[0].out||src_amp[1].out;
        if(enabled){sj6_amp_prep(&src_amp[0],m);sj6_amp_prep(&src_amp[1],m);}
        for(unsigned j=0;j<m;j++){
            amp_live[at+j]=enabled;
            for(unsigned i=0;i<2;i++)amp_tape[i][at+j]=enabled?sj6_amp_sample(&src_amp[i]):0.f;
        }
        at+=m;
    }
}
int sj6_amp_diagnostic_begin(const dnbx_part *s,unsigned n)
{
    if(!amp_scope||s!=&part.dsp.juno.synth.core)return 0;
    offset_words=consumed_words;
    consumed_words+=n;
    assert(consumed_words<=span_words);
    return 1;
}
float sj6_amp_diagnostic_value(unsigned l,unsigned k) {return amp_tape[l][offset_words+k];}
int sj6_amp_diagnostic_live(unsigned k){return amp_live[offset_words+k];}
static void amp_checked(unsigned n){if(amp_scope){assert(consumed_words==n);amp_words+=n;}}
