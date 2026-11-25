#define X0X_ARP_KEYBOARD_TEST 1
#include "gate_helpers.inc"
#include "noise_lifecycle_for_ui.h"
/* Original selected mode1 FILTER uses stages 1..5, independently of AMP. */
typedef struct {
 Segment g;
 unsigned stage, reset, attack, decay, release;
 double sustain;
} NoiseFilter;
static NoiseFilter nf[NBASS];
static double max_cut[NBASS];
static void nf_enter(NoiseFilter *f,unsigned stage){
 for(;;){
   Segment *g=&f->g; unsigned n; double dst,curve;
   switch(stage){
     case 1:n=f->reset;dst=0.;curve=0.;break;
     case 2:n=f->attack;dst=1.;curve=.22313016014842982;break;
     case 3:n=f->decay;dst=f->sustain;curve=.0070834089290521185;break;
     case 4:f->stage=4;g->here=f->sustain;return;
     case 5:n=f->release;dst=0.;curve=.0070834089290521185;break;
     default:f->stage=0;g->here=0.;return;
   }
   if(!n){g->here=dst;stage++;continue;}
   f->stage=stage;g->to=dst;g->waiting=(int)n;g->linear=curve==0.;
   if(g->linear){g->a=1.;g->b=(dst-g->here)/(double)n;}
   else {
     g->a=exp(-log((curve+fabs(dst-g->here))/curve)/(double)n);
     g->b=(dst+(dst>g->here?curve:-curve))*(1.-g->a);
   }
   return;
 }
}
static float nf_step(NoiseFilter *f){
 Segment *g=&f->g;
 if(!f->stage||f->stage==4) return (float)g->here;
 double x=g->linear?g->here+g->b:fma(g->a,g->here,g->b);
 if(--g->waiting==0 || (g->here<g->to ? x>=g->to:x<=g->to)){
   float endpoint=(float)g->to;g->here=g->to;nf_enter(f,f->stage+1);return endpoint;
 }
 g->here=x;return (float)x;
}
static double filter_cut(double base,double depth,double endpoint){
 base=fmax(1.,base);
 double upper=fmax(base,22050.),e=fmin(1.,fmax(0.,endpoint));
 double fraction=fmin(1.,fmax(0.,depth))*(1.-(1.-e)*(1.-e));
 double lo=log2(base);
 return exp2(fma(fraction,log2(upper)-lo,lo));
}
static Noise noise[NBASS];
static unsigned raw_saga[NBASS], observed_raw[NBASS], noise_on_count[NBASS], noise_off_count[NBASS];
static unsigned noise_note_plus_one[NBASS], route_admit[NBASS];
static B colors[3]; static float color_gain;
typedef struct { dnbx_release_times retained[2], effective[3]; unsigned destination,running; } SagaRelease;
static SagaRelease release_state[2];
static void saga_reconcile(SagaRelease *s,unsigned gate)
{
 const dnbx_release_times brief={.09f,.3f};
 if(!s->running || gate>=100u){
   s->effective[0]=s->retained[0];s->effective[1]=s->retained[1];s->effective[2]=s->retained[0];return;
 }
 switch(s->destination){
 case 0:s->effective[0]=s->effective[1]=s->effective[2]=brief;break;
 case 1:s->effective[0]=s->effective[2]=brief;s->effective[1]=s->retained[1];break;
 case 2:s->effective[0]=s->retained[0];s->effective[1]=brief;break; /* Noise no setter call. */
 }
}
int dnbox_host_sj6_release_times(const original_runtime_part *p,dnbx_release_times out[2]){
 for(unsigned o=0;o<NBASS;o++) if(parts_juno[o]==&p->dsp.juno.synth){
   saga_reconcile(&release_state[o],p->arp.cfg.gate);
   out[0]=release_state[o].effective[0];out[1]=release_state[o].effective[1];return 1;
 }
 return 0;
}
static unsigned osc_route[NBASS];
static int generated_zero[NBASS];
static unsigned hits[NBASS];
static uint32_t pads;
static uint64_t keys;
uint32_t plat_arp_input(uint64_t *out){*out=keys;return pads;}
static void gesture(uint32_t p,uint64_t k){
 pads=p;keys=k;held_keys=p|(uint32_t)(k>>5);
 ++now_ms;input();render(4);
}
unsigned dnbox_host_sj6_osc_route(const original_runtime_part *p,unsigned note,uint8_t vel[2]){
 (void)note;
 for(unsigned v=0;v<NBASS;v++) if(parts_juno[v]==&p->dsp.juno.synth && osc_route[v]){
   CHECK(vel[0]>0 && vel[0]==vel[1]);
   ++hits[v];
   if(generated_zero[v]) vel[0]=vel[1]=0;
   return osc_route[v];
 }
 return 0;
}
int dnbox_host_sj6_noise_on(const original_runtime_part *p,unsigned note){
 for(unsigned o=0;o<NBASS;o++) if(parts_juno[o]==&p->dsp.juno.synth && route_admit[o]){
   CHECK(raw_saga[o]<=127 && !noise_note_plus_one[o]);
   saga_reconcile(&release_state[o],p->arp.cfg.gate);
   observed_raw[o]=raw_saga[o];
   nf[o]=(NoiseFilter){.reset=1,.attack=8,.decay=12,.sustain=.42};
   nf_enter(&nf[o],1);
   on(&noise[o],note);noise_note_plus_one[o]=note+1;
   ++noise_on_count[o];
   return 1;
 }
 return 0;
}
int dnbox_host_sj6_noise_off(const original_runtime_part *p,unsigned note){
 for(unsigned o=0;o<NBASS;o++) if(parts_juno[o]==&p->dsp.juno.synth && noise_note_plus_one[o]==note+1){
   saga_reconcile(&release_state[o],p->arp.cfg.gate);
   /* Selected original sampler setter truncates binary32 frames/16. */
   noise[o].p.release=(unsigned)(release_state[o].effective[2].amp*44100.f/16.f);
   nf[o].release=(unsigned)(release_state[o].effective[2].filter*44100.f/16.f);
   nf[o].g.here=(float)nf[o].g.here;
   nf_enter(&nf[o],5);
   off(&noise[o],note);noise_note_plus_one[o]=0;++noise_off_count[o];return 1;
 }
 return 0;
}
static double step_noise(unsigned owner,unsigned until_release,unsigned blocks){
 double energy=0; unsigned n=0;
 for(;n<blocks && (!until_release || noise_note_plus_one[owner]);++n){
   const double cutoff=filter_cut(850.,.07,nf_step(&nf[owner]));
   max_cut[owner]=fmax(max_cut[owner],cutoff);
   CHECK(isfinite(cutoff)&&cutoff>=849.999&&cutoff<1200.);
   const Coeff cf=coefficients(cutoff,.7);
   prepare(&noise[owner],(float)observed_raw[owner]/127.0f);
   for(unsigned i=0;i<16;i++){
     float lr[2];
     rx_step(&noise[owner].dsp,colors,color_gain,&cf,lr);
     energy+=(double)lr[0]*lr[0]+(double)lr[1]*lr[1];
     if(until_release){ sample(1);if(!noise_note_plus_one[owner])break;}
   }
 }
 CHECK(!until_release || !noise_note_plus_one[owner]);
 return energy;
}
static unsigned stops[2];
int dnbox_host_sj6_generated_stop(const original_runtime_part *p){
 for(unsigned o=0;o<NBASS;o++) if(parts_juno[o]==&p->dsp.juno.synth){
   release_state[o].running=0;saga_reconcile(&release_state[o],p->arp.cfg.gate);
   route_admit[o]=0;++stops[o];return 1;
 }
 return 0;
}
static dnbx_voice *owned(unsigned v,unsigned note){
 for(unsigned i=0;i<DNBX_VOICES;i++){
   dnbx_voice *q=&parts_juno[v]->core.voice[i];
   if(q->osc_route && q->note==note)return q;
 }
 return NULL;
}
static void grid(unsigned v){
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_SEQ);
 *page_of(V_SEQ)=3;build_pages();smc_peek_cancel();ui.oct=0;
 CHECK(arp_seq_page());
 uint32_t pad=1u<<WHITE_KEY[0];
 uint64_t alias=UINT64_C(1)<<(WHITE_KEY[0]+5);
 gesture(0,0);gesture(pad,0);gesture(pad,alias);gesture(0,alias);gesture(0,0);
 engine_arp_steps st;CHECK(engine_arp_steps_read(v,&st));fprintf(stderr,"part=%u offset=%d mask=%u\n",v,st.offset[0],st.on_mask);CHECK(st.offset[0]==-7&&(st.on_mask&1u));
}
static void stop_test(unsigned owner){
    unsigned peer=owner^1u;
    SagaRelease *sr=&release_state[owner];
    set_arp(owner,ARP_F_MODE,ARP_UP);
    set_arp(owner,ARP_F_ON,1);set_arp(owner,ARP_F_DIVISION,1);
    set_arp(owner,ARP_F_GATE,51);
    select_page_gate(owner);gate_number(51);detent(2,-1);render(1);gate_number(50);
    sr->destination=1;sr->running=1;
    raw_saga[owner]=96;osc_route[owner]=1;route_admit[owner]=1;generated_zero[owner]=1;
    noise[owner]=(Noise){.p={8,2,12,6,40,.75f},.raw=1.f};
    dnbx_part *same=&parts_juno[owner]->core,*other=&parts_juno[peer]->core;
    int manual=dnbx_note_on(same,72,100), peer_manual=dnbx_note_on(other,73,100);
    CHECK(manual>=0 && peer_manual>=0 && same->voice[manual].gate && other->voice[peer_manual].gate);
    seq_start(&seq);sample(50);
    engine_bass_on_velocity(owner,60,0,0,100);
    for(unsigned i=0;i<1500 && !noise_note_plus_one[owner];++i)sample(1);
    dnbx_voice *q=owned(owner,60);
    CHECK(seq.playing && q && q->osc_live==1 && noise_note_plus_one[owner]==61);
    CHECK(sr->effective[0].amp==.09f && sr->effective[2].amp==.09f);
    sample(9);CHECK(q->osc_live==1);
    CHECK(step_noise(owner,0,30)>0 && nf[owner].stage==4);
    unsigned off_before=noise_off_count[owner],peer_before=noise_off_count[peer],n=stops[owner];
    seq_stop(&seq);sample(1);
    CHECK(!seq.playing && stops[owner]==n+1 && !q->osc_live);
    fprintf(stderr,"STOP o=%u off=%u/%u peer=%u/%u noteplus=%u\n",owner,noise_off_count[owner],off_before,noise_off_count[peer],peer_before,noise_note_plus_one[owner]);
    CHECK(noise_off_count[owner]==off_before+1 && noise_off_count[peer]==peer_before);
    CHECK(q->saga_amp_release[0]==.133f && q->saga_filter_release[0]==.411f);
    CHECK(noise[owner].p.release==(unsigned)(.133f*44100.f/16.f));
    CHECK(nf[owner].stage==5 && nf[owner].release==(unsigned)(.411f*44100.f/16.f));
    float before=nf[owner].g.here;
    CHECK(before>.4f && nf_step(&nf[owner])<before && nf[owner].stage==5);
    CHECK(same->voice[manual].gate && same->voice[manual].note==72 &&
          other->voice[peer_manual].gate && other->voice[peer_manual].note==73);
    engine_bass_off_note(owner,60);sample(3);
    CHECK(noise_off_count[owner]==off_before+1 && !q->osc_live);
    dnbx_note_off(same,72);dnbx_note_off(other,73);
    osc_route[owner]=0;
}
int main(void){
 init();config(colors,&color_gain);
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 stage_juno(1); seq.bpm=120.f;render(2);
 for(unsigned v=0;v<NBASS;v++){
   SagaRelease *sr=&release_state[v];
   sr->retained[0]=(dnbx_release_times){.133f,.411f};
   sr->retained[1]=(dnbx_release_times){.197f,.522f};
   for(unsigned m=0;m<2;m++){
     sr->running=0;saga_reconcile(sr,50);CHECK(sr->effective[2].amp==.133f);
     sr->running=1;sr->destination=m?2:1;
     saga_reconcile(sr,100);CHECK(sr->effective[2].filter==.411f);
     saga_reconcile(sr,99);CHECK(sr->effective[2].amp==(m?.133f:.09f));
     if(!m){
        sr->destination=0;saga_reconcile(sr,50);CHECK(sr->effective[0].amp==.09f&&sr->effective[1].filter==.3f);
        sr->destination=1;saga_reconcile(sr,50);CHECK(sr->effective[1].amp==.197f);
     }
     if(m)grid(v);
     set_arp(v,ARP_F_MODE,m?ARP_SEQ:ARP_UP);
     set_arp(v,ARP_F_ON,1);set_arp(v,ARP_F_DIVISION,1);set_arp(v,ARP_F_GATE,51);
     select_page_gate(v);gate_number(51);detent(2,-1);render(1);gate_number(50);
     unsigned selected=1u<<v,note=m?53:60,previous=hits[v];
     unsigned before_on=noise_on_count[v],before_off=noise_off_count[v],peer_on=noise_on_count[1-v];
     noise[v]=(Noise){.p={8,2,12,6,40,.75f},.raw=1.f};
     Noise other=noise[1-v];
     osc_route[v]=selected;generated_zero[v]=!m;route_admit[v]=1;raw_saga[v]=83u+v;
     engine_bass_on_velocity(v,60,0,0,100);
     sample(1);dnbx_voice *q=owned(v,note);
     CHECK(hits[v]==previous+1 && q && q->osc_live==selected);
     CHECK(noise_on_count[v]==before_on+1 && noise_note_plus_one[v]==note+1);
     CHECK(observed_raw[v]==83+v && observed_raw[v]!=100 && observed_raw[v]!=0);
     CHECK((v?q->generated_velocity2:q->velocity)==(m?100.f/127.f:0.f));
     const float retained_core=parts_juno[v]->core.layer[v].params.amp.release_s;
     route_admit[v]=0;raw_saga[v]=2;osc_route[v]=selected^3u;
     sr->destination=2;
     double on_energy=step_noise(v,1,80);
     CHECK(q->osc_live==0 && !q->gate && noise_off_count[v]==before_off+1);
     CHECK(noise[v].stage>=6 && observed_raw[v]==83+v && on_energy>0);
     const float expected_noise=m?.133f:.09f;
     CHECK(noise[v].p.release==(unsigned)(expected_noise*44100.f/16.f));
     CHECK(nf[v].stage==5 && nf[v].release==(unsigned)((m?.411f:.3f)*44100.f/16.f));
     CHECK(q->saga_release_mask==selected && q->saga_amp_release[v]==(v?.09f:.133f));
     CHECK(q->saga_filter_release[v]==(v?.3f:.411f));
     CHECK(q->layer[v].amp.stage==DNBX_ENV_RELEASE);
     CHECK(q->layer[v].amp.remaining+q->layer[v].amp.tick_left+1==(unsigned)(q->saga_amp_release[v]*parts_juno[v]->core.sample_rate+.5f));
     CHECK(q->layer[v].filter.stage==DNBX_ENV_RELEASE);
     CHECK(q->layer[v].filter.remaining+q->layer[v].filter.tick_left+1==(unsigned)(q->saga_filter_release[v]*parts_juno[v]->core.sample_rate+.5f));
     CHECK(parts_juno[v]->core.layer[v].params.amp.release_s==retained_core);
     sr->running=0;saga_reconcile(sr,50);CHECK(sr->effective[2].amp==.133f);
     CHECK(q->saga_amp_release[v]==(v?.09f:.133f)); /* prior release is still latched */
     engine_bass_off_note(v,60);sample(3);
     CHECK(!all_gates(parts_juno[v]) && noise_off_count[v]==before_off+1);
     double tail_energy=step_noise(v,0,550);
     CHECK(tail_energy>0 && !noise[v].stage && nf[v].stage==5);
     CHECK(step_noise(v,0,800)<1e-16 && !nf[v].stage && max_cut[v]>900.);
     CHECK(noise_on_count[1-v]==peer_on && !memcmp(&other,&noise[1-v],sizeof other));
     osc_route[v]=0;
   }
 }
 for(unsigned o=0;o<NBASS;o++)stop_test(o);
 printf("PASS %u UBSan selected Noise FILTER independently changes live LP2 cutoff and stays active after AMP retirement; actual transport STOP restores releases before real generated off; independent unrelated notes survive; OSC AMP/FILTER short-release timing and Noise AMP retention after actual generated offs, both owners; destination2 no-call, gate=100 and STOP restore\n",checks);
}
