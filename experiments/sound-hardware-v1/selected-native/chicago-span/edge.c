#define main stock_span_test_main
#include "chicago_span_hold_test.c"
#undef main
static unsigned nfall, nlate, nwide;
static void edge(unsigned single,unsigned shape,unsigned mode,uint32_t inc,unsigned count){
 original_prepared_oscillators p; original_voice_gain vg={0}; original_oscillators ph={0};
 original_chicago_mp4 f={0}; original_ramp ng; uint32_t ns=rng();
 fill_prep(&p,shape,inc,.5f,0.0f); f.prepared=1;
 for(unsigned i=0;i<4;++i){
   p.controls.shape[i]=shape; p.controls.width[i]=.5f;p.correction[i]=.375f;p.triangle[i]=.4f;
   p.controls.increment[i]=(i&1u)?rng():inc;
   ph.phase[i]=rng();held(&vg.oscillator[i],i==single?.83203125f:0.0f);
 }
 held(&ng,.125f);held(&vg.stereo[0],.5f);held(&vg.stereo[1],.625f);
 held(&f.a,.513671875f);held(&f.e,.25f);held(&f.resonance,.75f);
 if(mode==1)set_ramp(&f.a,.5f,.75f,.00390625f);
 if(mode==2)set_ramp(&vg.stereo[0],.5f,.75f,.00390625f);
 if(mode==3)set_ramp(&vg.oscillator[single],.5f,.75f,.00390625f);
 if(mode==4){held(&f.a,2.5f);f.history[0]=1.f;} /* delayed overflow, after a valid PCM prefix */
 if(mode==5){held(&f.a,2.5f);f.history[0]=fl(0x7f700000u);} /* first-frame overflow */
 if(mode==6)set_ramp(&f.e,.25f,.75f,.00000000001f);
 if(mode==7){held(&f.a,-0.0f);held(&f.resonance,-0.0f);}
 if(mode>=8){
  set_ramp(&vg.stereo[0],.25f,.625f,.00390625f);
  vg.stereo[1]=vg.stereo[0];
  if(mode==9)set_ramp(&vg.oscillator[single],.83203125f,0,-.013f);
  if(mode==10){held(&f.a,2.5f);f.history[0]=1.f;}
  if(mode==11){set_ramp(&ng,.125f,.25f,.000625f);vg.stereo[1].target=.8125f;}
 }
 original_voice_gain vg2=vg;original_oscillators ph2=ph;original_chicago_mp4 f2=f;
 original_ramp ng2=ng;uint32_t ns2=ns,L[257],R[257],X[257],Y[257];
 for(unsigned i=0;i<count;i++)L[i]=R[i]=X[i]=Y[i]=0x7fcddde0;
 int ok=original_chicago_voice_span(&vg,&ph,&p,&ng,&ns,1.25f,&f,L,R,count),pass=1;
 unsigned j;
 for(j=0;j<count;j++)if(!ref_step(&p,1.25f,ph2.phase,vg2.oscillator,vg2.stereo,&ng2,&ns2,f2.history,&f2.a,&f2.e,&f2.resonance,X+j,Y+j)){pass=0;break;}
 if(inc>INT32_MAX) nwide++;
 if(!pass) {nfall++;if(j>0)nlate++;}
 if (ok!=pass || memcmp(L,X,4*count) || memcmp(R,Y,4*count) ||
  memcmp(&vg,&vg2,sizeof vg) || memcmp(&ph,&ph2,sizeof ph) || memcmp(&ng,&ng2,sizeof ng) || ns!=ns2 || memcmp(&f,&f2,sizeof f)){
  fprintf(stderr,"single=%u shape=%u mode=%u inc=%08x count=%u accepted=%d/%d prefix=%u controls=%d phases=%d filter=%d\n",single,shape,mode,inc,count,ok,pass,j,memcmp(&vg,&vg2,sizeof vg),memcmp(&ph,&ph2,sizeof ph),memcmp(&f,&f2,sizeof f));exit(1);
 }
 checks++;
}
int main(void){
 const uint32_t increments[]={0,1,65521,0x20000001u,0x7fffffffu,0x80000000u,0x80000001u,0xf0000003u,0xffffffffu};
 const unsigned sizes[]={1,2,13,31,32,137,256,257};
 for(unsigned s=0;s<4;s++)for(unsigned shape=0;shape<4;shape++)for(unsigned mode=0;mode<12;mode++)
 for(unsigned i=0;i<sizeof increments/sizeof*increments;i++)for(unsigned n=0;n<sizeof sizes/sizeof*sizes;n++)edge(s,shape,mode,increments[i],sizes[n]);
 printf("%u exact PCM, full DSP state and untouched suffix comparisons; %u increments > INT32_MAX; %u failures, %u after nonempty PCM\n",checks,nwide,nfall,nlate);
}
