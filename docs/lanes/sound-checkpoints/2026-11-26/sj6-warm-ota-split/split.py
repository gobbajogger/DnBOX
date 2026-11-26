from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def put(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
put('static inline float sj6_ota_stage', '''/* One single-instance host comparison. Not a production state owner. */
static float host_ota_sub[DNBX_VOICES][4];
static unsigned ota_split,split_count;
__attribute__((visibility("default"))) void ota_split_enable(unsigned x) { ota_split=x; split_count=0; memset(host_ota_sub,0,sizeof host_ota_sub); }
__attribute__((visibility("default"))) unsigned ota_split_count(void) { return split_count; }
static inline float sj6_ota_stage''')
put('witness_reset(chosen);','witness_reset(chosen); if(ota_split)memset(host_ota_sub[chosen],0,sizeof host_ota_sub[chosen]);')
put('                if (osc_gain[l]!=1.f) x*=osc_gain[l];','''                float separate_sub=0.f;
                const int split_sub=ota_split && s->superj6_amp_range && s->sj6_ota &&
                   !s->sj6_noise_gain && s->sj6_sub_gain && !authored_track &&
                   authored_strength && strength==0.f &&
                   s->layer[0].params.oscillator_mode==DNBX_OSC_SUPERJ6 &&
                   s->layer[1].params.oscillator_mode==DNBX_OSC_SUPERJ6 &&
                   s->layer[0].params.sub_gain==0.f && s->layer[1].params.sub_gain==0.f;
                if (!split_sub && osc_gain[l]!=1.f) x*=osc_gain[l];''')
put('x+=s->sj6_sub_gain*shape_carrier(k,i,v->gate,amp,c);','if(split_sub)separate_sub=c;else x+=s->sj6_sub_gain*shape_carrier(k,i,v->gate,amp,c);')
put('                if(s->sj6_ota)mix[l]+=velocity*amp*sj6_ota_filter(lv,x,a,p->resonance);','''                if(s->sj6_ota) {
                    float y=sj6_ota_filter(lv,x,a,p->resonance);
                    if(!split_sub)mix[l]+=velocity*amp*y;
                    else {
                      /* Receiver's faders belong after respective nonlinear states. */
                      mix[l]+=velocity*amp*(y*osc_gain[l]);
                      if(!l){
                        float *h=host_ota_sub[i];
                        const float g=.5f*a/(1.f+.5f*a);
                        float sub=separate_sub-(3.85f*fm_clampf(p->resonance,0.f,1.f))*h[3];
                        for(unsigned j=0;j<4;j++)sub=sj6_ota_stage(sub,h+j,g);
                        extern float shape_carrier(unsigned,unsigned,unsigned,float,float);
                        /* Keep prefilter source carrier raw; derive existing source-vs-stock release ratio without retuning. */
                        const float release=shape_carrier(k,i,v->gate,amp,1.f);
                        mix[l]+=velocity*amp*(s->sj6_sub_gain*sub*release);
                        split_count++;
                      }
                    }
                }''')
p.write_text(s)
