#include "juno6.h"
#include "filter_panel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct event {unsigned time,key,velocity;};
#include "case.h"
static juno6_t j;
#define s (j.core)
static float pcm[2][2][148684], tap[2][3][148684];
static int allow_tap,after;

static void render(unsigned oracle,unsigned split,float dest[2][148684],FILE *trace)
{
    juno6_init(&j,44100.f);
    juno6_mono32_set(&j,1u);
    juno6_legato33_set(&j,0u);
    juno6_set_sj6_hpf(&j,0.f);
    assert(juno6_set(&j,2,.4f));
    juno6_apply_pre_note_range(&j,0.f);
    juno6_set_layer_semi(&j,0,12);
    juno6_set_layer_semi(&j,1,0);
    juno6_set_a_reference(&j,.6000000238418579f,1u);
    assert(juno6_glide35_set(&j,61u));
    /* Authored Float32 glide sends a Double-scaled .5 to the host. */
    j.core.sj6_glide_secs=(float)((double).4823414087295532f*.5);
    assert(juno6_configure_lfo1_cutoff(&j,.3345831036567688f,.3345831036567688f,.09651908278465271f));
    assert(juno6_set(&j,3,.4141378700733185f));
    assert(dnbx_store_raw(&s,136,.4559931755065918f)==DNBX_RAW_STORED_ONLY);
    /* Detents retain route carrier compatibility. Actual computation uses
       the original raw Float32 amount/base words for the two loaded routes. */
    j.host_lfo_words_valid=1u;
    j.host_lfo_amount[1]=.3197075426578522f;
    j.host_lfo_amount[2]=.9787701368331909f;
    j.host_lfo_base[1]=.4141378700733185f;
    j.host_lfo_base[3]=.4559931755065918f;
    uint8_t aux[sizeof j.aux_lfo]={0};
    aux[0]=aux[1]=2u;
    aux[2]=124u; aux[3]=SJ6_LFO_UP;
    aux[5]=45u; aux[7]=22u; /* authored target indices 38 and 15 */
    aux[9]=53u; aux[11]=58u;
    juno6_set_aux_lfo(&j,aux);
    juno6_set_lfo2_pwm(&j,41u);
    juno6_set_lfo2_shape(&j,SJ6_LFO_DOWN);
    const juno6_lfo_words lfo={.word={.07960975915193558f,.2692681849002838f,.2411191761493683f,1.f,0.f,1.f},.present=63};
    juno6_set_lfo_tempo(&j,120.f);
    juno6_set_lfo_words(&j,&lfo,1);
    juno6_lfo1_delay_set(&j,45u); /* retain existing enabled/pending state */
    const float raw=.35310420393943787f;
    const float seconds=(float)(3.*pow((double)raw,1.21));
    const unsigned ticks=(unsigned)(seconds*80.f)+1u;
    j.lfo1_delay_frames=(ticks*44100u+79u)/80u;
    for(unsigned l=0;l<2;l++) {
        dnbx_layer_params p;
        assert(dnbx_get_layer(&s,l,&p)==DNBX_OK);
        p.oscillator_mode=DNBX_OSC_SUPERJ6;
        p.waveform=2.f/3.f;
        p.sub_gain=0;
        p.amp.attack_s=sj6_amp_panel(capture_amp[l][0],.002,16.,3.33);
        p.amp.decay_s=sj6_amp_panel(capture_amp[l][1],.002,24.,3.35);
        p.amp.sustain=sj6_amp_panel(capture_amp[l][2],0,1,2.5);
        p.amp.release_s=sj6_amp_panel(capture_amp[l][3],.002,24.,3.35);
        assert(dnbx_set_layer(&s,l,&p)==DNBX_OK);
    }
    if(oracle) {
        assert(dnbx_host_main_oracle_setup(&s,capture_amp)==DNBX_OK);
        /* Scoped source-proven panel input trace. Base and damping are
           fixture values until independent original owner readbacks land. */
        const dnbx_host_sj6_normal_inputs authored={
            .base_hz=300.f,.damping=sj6_filter_damping_panel(.4141378700733185f),.follow_provider_cutoff=1,.follow_provider_resonance_strength=1,
            .attack_s=sj6_filter_attack_panel(.5168501734733582f),
            .decay_s=sj6_filter_decay_panel(.47146177291870117f),
            .sustain=sj6_filter_sustain_panel(.08876017481088638f),
            .release_s=sj6_filter_release_panel(.604790449142456f),
            .strength=sj6_filter_strength_panel(.4559931755065918f),
            .track=sj6_filter_tracking_panel(.5313359498977661f),
            .velocity_amount=sj6_filter_velocity_panel(.9064822196960449f)
        };
        s.main_oracle.ablate_scale_inside_halfband=after;
        if(allow_tap){
            s.main_oracle.tap_len=148684;
            s.main_oracle.main_filtered[0]=tap[split][0];
            s.main_oracle.main_filtered[1]=tap[split][1];
            s.main_oracle.osc_sum=tap[split][2];
        }
        for(unsigned l=0;l<2;l++){
            assert(dnbx_host_main_normal_dynamic(&s,l,authored)==DNBX_OK);
            assert(dnbx_host_main_source_gain(&s,l,.808f)==DNBX_OK);
        }
    }
    unsigned t=0,e=0;
    for(unsigned b=0;b<sizeof capture_blocks/sizeof *capture_blocks;b++){
        unsigned n=capture_blocks[b];
        while(e<sizeof capture_events/sizeof *capture_events && capture_events[e].time==t){
            const struct event q=capture_events[e++];
            const double state=s.main_oracle.normal_hist[0].y1;
            if(q.velocity) assert(juno6_note_on(&j,q.key,86u));
            else juno6_note_off(&j,q.key);
            if(oracle && t==49459){
                assert(state!=0.0 && s.main_oracle.normal_hist[0].y1==state);
                assert(s.main_oracle.normal_hist[0].y1==state);
            }
        }
        s.main_oracle.cb=(sj6_source_cb){t,n};
        for(unsigned k=0;k<n;){
            unsigned step=split?3u+k%7u:n;
            unsigned m=step<n-k?step:n-k;
            assert(juno6_render_stereo(&j,dest[0]+t+k,dest[1]+t+k,m));
            k+=m;
        }
        t+=n;
    }
    assert(t==148684 && e==sizeof capture_events/sizeof *capture_events);
    fprintf(trace,"oracle=%u split=%u commit=%llu,%llu\n",oracle,split,
           (unsigned long long)s.main_oracle.commit[0],(unsigned long long)s.main_oracle.commit[1]);
    if(oracle) {
        assert(s.main_oracle.commit[0]==49875 && s.main_oracle.commit[1]==49875);
        for(unsigned l=0;l<2;l++){
            const float lo=s.main_oracle.min_provider_base[l], hi=s.main_oracle.max_provider_base[l];
            fprintf(trace,"Main%u provider cutoff %.6f..%.6f Hz\n",l+1,(double)lo,(double)hi);
            assert(lo<hi);
            const float k=s.main_oracle.min_damping[l],K=s.main_oracle.max_damping[l];
            const float a=s.main_oracle.min_strength[l],A=s.main_oracle.max_strength[l];
            fprintf(trace,"Main%u modulated damping %.8f..%.8f, strength %.8f..%.8f\n",
                    l+1,(double)k,(double)K,(double)a,(double)A);
            assert(k<K && a<A);
        }
    }
    for(unsigned j=0;j<t;j++) assert(isfinite(dest[0][j])&&isfinite(dest[1][j]));
}
int main(int argc,char **argv){
    allow_tap=1;
    assert(argc<=3);
    after=argc==3 && strcmp(argv[2],"pre")==0;
    render(1,0,pcm[0],stdout);
    render(1,1,pcm[1],stdout);
    /* Changing the outer provider's partition is allowed to affect its
       existing LFO implementation; the explicit Main clock is still exact. */
    assert(memcmp(pcm[0],pcm[1],sizeof pcm[0])==0);
    assert(memcmp(tap[0],tap[1],sizeof tap[0])==0);
    assert(tap[0][0][16384]!=0 && tap[0][1][16384]!=0);
    assert(tap[0][2][16383]==0 && tap[0][2][16384]!=0);
    if(argc>=2){
        FILE *out=fopen(argv[1],"wbx");
        assert(out);
        assert(fwrite(tap[0],1,sizeof tap[0],out)==sizeof tap[0]);
        assert(fclose(out)==0);
    }
    render(0,0,pcm[1],stdout);
    assert(memcmp(pcm[0],pcm[1],sizeof pcm[0])!=0);
    puts("PASS: typed factory provider, recorded callbacks, typed provider with live source normalization and tapped independent Main lanes");
}
