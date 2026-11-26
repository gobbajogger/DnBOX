from pathlib import Path
import sys
q,z=map(Path,sys.argv[1:])
s=q.read_text()
def change(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
change('static inline float sj6_ota_stage(float input,float *state,float g)','''/* Host tape holds one independent .OTA sub state for the single fixture owner. */
static float host_ota_sub[DNBX_VOICES][4];
static inline float sj6_ota_stage(float input,float *state,float g)''')
change('    if (!retrigger) memset(v,0,sizeof *v);','''    if (!retrigger){
        memset(v,0,sizeof *v);
        if(HOST_OTA_SPLIT_SUB)memset(host_ota_sub[chosen],0,sizeof host_ota_sub[chosen]);
    }''')
change('                const int gain_after=HOST_OTA_GAIN_AFTER &&','''                float separate_sub=0.f;
                const int split_sub=HOST_OTA_SPLIT_SUB && s->superj6_amp_range && s->sj6_ota &&
                   !s->sj6_noise_gain && s->sj6_sub_gain && !authored_track &&
                   authored_strength && strength==0.f &&
                   s->layer[0].params.oscillator_mode==DNBX_OSC_SUPERJ6 &&
                   s->layer[1].params.oscillator_mode==DNBX_OSC_SUPERJ6 &&
                   s->layer[0].params.sub_gain==0.f && s->layer[1].params.sub_gain==0.f;
                const int gain_after=(HOST_OTA_GAIN_AFTER || split_sub) &&''')
change('p->sub_gain==0.f && !s->sj6_noise_gain && !s->sj6_sub_gain;','p->sub_gain==0.f && !s->sj6_noise_gain && (!s->sj6_sub_gain || split_sub);')
change('                        x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);','''                        const float carrier=sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);
                        if(split_sub)separate_sub=carrier;
                        else x+=s->sj6_sub_gain*carrier;''')
change('                    mix[l]+=velocity*amp*y;','''                    if(split_sub && !l){
                        float *h=host_ota_sub[i];
                        const float g=.5f*a/(1.f+.5f*a);
                        float h0=separate_sub-(3.85f*fm_clampf(p->resonance,0.f,1.f))*h[3];
                        for(unsigned j=0;j<4;j++)h0=sj6_ota_stage(h0,h+j,g);
                        y+=s->sj6_sub_gain*h0;
                        extern unsigned split_sub_count;
                        split_sub_count++;
                    }
                    mix[l]+=velocity*amp*y;''')
z.write_text(s)
