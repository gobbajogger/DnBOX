from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def put(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
put('static inline float sj6_ota_stage','''#include "physical.c"
static OTA main_ota[DNBX_VOICES];
static dnbx_layer_voice isolated_stock[DNBX_VOICES], isolated_sub[DNBX_VOICES];
static float coeff_ota[5], params_ota[4];
static unsigned ota_arm,ota_count;
__attribute__((visibility("default"))) void select_ota(unsigned arm,const float *p,const float *c) {
    assert(arm<=2);
    ota_arm=arm;ota_count=0;
    memcpy(params_ota,p,sizeof params_ota);memcpy(coeff_ota,c,sizeof coeff_ota);
    memset(main_ota,0,sizeof main_ota);memset(isolated_stock,0,sizeof isolated_stock);
    memset(isolated_sub,0,sizeof isolated_sub);
}
__attribute__((visibility("default"))) unsigned ota_frames(void) {return ota_count;}
static inline float sj6_ota_stage''')
put('if (!retrigger) memset(v,0,sizeof *v);','''if (!retrigger) {
    memset(v,0,sizeof *v);
    if(ota_arm){memset(main_ota+chosen,0,sizeof *main_ota);memset(isolated_stock+chosen,0,sizeof *isolated_stock);memset(isolated_sub+chosen,0,sizeof *isolated_sub);}
    }''')
put('                if (osc_gain[l]!=1.f) x*=osc_gain[l];','''                const float main_in=x;
                float sub_in=0.f;
                if (osc_gain[l]!=1.f) x*=osc_gain[l];''')
put('                        x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);','''                        sub_in=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);
                        x+=sub_in;''')
put('                if(s->sj6_ota)mix[l]+=velocity*amp*sj6_ota_filter(lv,x,a,p->resonance);','''                if(s->sj6_ota){
                    float y=sj6_ota_filter(lv,x,a,p->resonance);
                    if(ota_arm && !l && s->superj6_amp_range && !s->sj6_noise_gain &&
                      !p->sub_gain && p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->pan==0.f &&
                      a==.75f && p->resonance==0.f && osc_gain[0]>0.f) {
                       const float orig=physical_tick(main_ota+i,main_in,params_ota,coeff_ota)*osc_gain[0];
                       if(ota_arm==1) {
                           const float m=sj6_ota_filter(isolated_stock+i,main_in*osc_gain[0],a,p->resonance);
                           y += orig-m;
                       } else {
                           y=orig+sj6_ota_filter(isolated_sub+i,sub_in,a,p->resonance);
                       }
                       ++ota_count;
                    }
                    mix[l]+=velocity*amp*y;
                }''')
p.write_text(s)
d=Path(sys.argv[2]);ss=d.read_text();aa='int main(int argc,char**argv)';assert ss.count(aa)==1;d.write_text(ss.replace(aa,'__attribute__((visibility("default"))) int run_render(int argc,char**argv)'))
