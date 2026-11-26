#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint32_t rng=82;
static float sample(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return ((int32_t)rng)*0x1p-32f;}
int main(void){
 GapA a={0};GapB b={0};GapC c={0};ColdLease l={&a,&b,&c};
 dnbx_voice voices[6]={0};OTA baseline[6]={0};
 const float p[4]={1.821f,.000356f,1.f,1.f}, k[5]={.22f,.502f,.10f,.88f,.025f};
 ColdProfile d={30000.f,44100.f,1.191f,.808f,p,k};
 unsigned checked=0; ColdRun run={&d,1,0};
 for(unsigned i=0;i<6;i++)reset_cold(l,voices+i,i);
 for(unsigned n=0;n<65536;n++){
  const unsigned i=n%6;
  if(n==4099){reset_cold(l,voices+i,i);baseline[i]=(OTA){0};}
  float in=sample(),sub=sample();
  FrameFingerprint f={d.board_hz,d.rate,0.f,in,d.gain,2.f/3.f,DNBX_OSC_SUPERJ6,1,1,0,0,1,1};
  float y=0;
  const float ref=(physical_tick(baseline+i,in,p,k)*d.gain)+sub;
  assert(guarded_main_sample(&run,l,voices+i,i,&f,sub,&y)==1 && !memcmp(&ref,&y,4));checked++;
  for(unsigned j=0;j<10;j++){
    GapA a0=a;GapB b0=b;GapC c0=c; dnbx_voice prev[6];memcpy(prev,voices,sizeof prev);
    FrameFingerprint e=f;
    switch(j){case 0:e.enabled=0;break;case 1:e.pitch_routed=1;break;case 2:e.pitch_depth=1;break;case 3:e.physical_hz=8510.41f;break;
     case 4:e.feedback=.001f;break;case 5:e.no_noise=0;break;case 6:e.main=d.peak_limit*2;break;case 7:e.ota=0;break;case 8:e.source_family=0;break;default:e.no_legacy_sub=0;}
    ColdRun bad={&d,j!=0,0};
    y=sub; assert(guarded_main_sample(&bad,l,voices+i,i,&e,sub,&y)==(j==0?0:-1) && !memcmp(&y,&sub,4));
    assert(guarded_main_sample(&bad,l,voices+i,i,&f,sub,&y)==(j==0?0:-1) && !memcmp(&y,&sub,4));
    assert(!memcmp(&a,&a0,sizeof a)&&!memcmp(&b,&b0,sizeof b)&&!memcmp(&c,&c0,sizeof c)&&!memcmp(voices,prev,sizeof prev));
  }
  for(unsigned v=0;v<6;v++)for(unsigned j=0;j<DNBX_LAYERS;j++)assert(voices[v].layer[j].filter_first==0.f && voices[v].layer[j].filter_second==0.f && voices[v].layer[j].ota_tail[0]==0.f && voices[v].layer[j].ota_tail[1]==0.f);
 }
 printf("%u exact signed typed split vs contiguous, %u rejects preserve all owners/fallback, both stock layers untouched\n",checked,10*checked);
}
