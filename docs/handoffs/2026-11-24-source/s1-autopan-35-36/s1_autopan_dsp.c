#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "soundpipe.h"
static s1c *watch;
static sp_data sp;
static sp_ftbl table;
static sp_osc osc;
static sp_pan2 pan;
static float sine_original[4097];
static unsigned seen,left,right,nonzero,zero_mono;
static double maxgain,maxpower;
static int active;
void __real_s1u_autopan_authored(float*,float,float,float,float,float*,float*);
void __wrap_s1u_autopan_authored(float *ph,float mono,float rate,float amount,float sr,float *l,float *r)
{
 if(!active || !watch || ph!=&watch->pan_ph) {
  __real_s1u_autopan_authored(ph,mono,rate,amount,sr,l,r);return;
 }
 CHECK((uint32_t)(*ph*16777216.f)==(uint32_t)osc.lphs);
 osc.freq=rate;osc.amp=amount;float ol,or_,orig;
 CHECK(sp_osc_compute(&sp,&osc,NULL,&orig)==SP_OK);pan.pan=orig;
 CHECK(sp_pan2_compute(&sp,&pan,&mono,&ol,&or_)==SP_OK);
 __real_s1u_autopan_authored(ph,mono,rate,amount,sr,l,r);
 CHECK((uint32_t)(*ph*16777216.f)==(uint32_t)osc.lphs);
 if(mono==0.f){zero_mono++;CHECK(*l==0.f&&*r==0.f);}
 if(fabsf(mono)>1.e-5f){
   double e=fmax(fabs((*l-ol)/mono),fabs((*r-or_)/mono));
   double power=fabs((double)(*l/mono)*(*l/mono)+(double)(*r/mono)*(*r/mono)-1.);
   if(e>maxgain)maxgain=e;if(power>maxpower)maxpower=power;
   CHECK(e<.00016&&power<.00019);++nonzero;
   if(orig>.2f){CHECK(fabsf(*r)>fabsf(*l));right++;}
   if(orig<-.2f){CHECK(fabsf(*l)>fabsf(*r));left++;}
 }
 seen++;
}
static float original_hz(float bpm,unsigned j){
 static const float bars[]={8,6,4,3,2,1,1,.5,.5,.25,.25,.125,.125,.0625,.0625,.03125,.03125,.015625,.015625};
 float seconds=bars[j]*4.f/(bpm*(1.f/60.f));
 if(j==6 || (j>=8 && !(j&1)))seconds/=1.5f;
 return 1.f/seconds;
}
static void actual_rate_oracle(s1c *s) {
 CHECK(s1c_front_set(s,139,0.f));
 CHECK(s1c_front_pan_stops(s)==127);
 for(unsigned i=0;i<=127;i++){
  const float lo=1.f/1920.f, x=(float)i/127.f;
  CHECK(s1c_front_pan_select(s,i));
  CHECK(fabsf(s->t[36]-(lo+(10.f-lo)*powf(x,2.5f)))<5.e-6f);
 }
 for(unsigned i=0;i<4;i++){
   const float bpms[]={30,90,120,200}, bpm=bpms[i];
   CHECK(s1c_front_set(s,64,bpm)&&s1c_front_set(s,139,1.f));
   unsigned last=0;while(last<18&&original_hz(bpm,last+1)<=10.f)last++;
   CHECK(s1c_front_pan_stops(s)==last);
   for(unsigned j=0;j<=last;j++){
     CHECK(s1c_front_pan_select(s,j));CHECK(fabsf(s->t[36]-original_hz(bpm,j))<=1.e-6f);
   }
   CHECK(!s1c_front_pan_select(s,last+1));
 }
}
static void oracle_init(s1c *e){
 memset(&sp,0,sizeof sp);memset(&table,0,sizeof table);memset(&osc,0,sizeof osc);memset(&pan,0,sizeof pan);
 sp.sr=e->sr;table.size=4096;table.sicvt=(float)(16777216.0/sp.sr);
 table.tbl=sine_original;table.lobits=12;table.lomask=4095;table.lodiv=1.f/4096.f;
 const float step=(float)(2.0*3.14159265358979323846/4096.);
 for(unsigned j=0;j<4096;j++)sine_original[j]=(float)sin(j*step);
 CHECK(sp_osc_init(&sp,&osc,&table,0)==SP_OK&&sp_pan2_init(&sp,&pan)==SP_OK);
 osc.lphs=(int32_t)(e->pan_ph*16777216.f);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();const sound_t original=proj.sound;const unsigned writes=store_writes;
 const char *dump=getenv("S1_AUTOPAN_UNTOUCHED");
 if(dump){engine_bass_on(0,60,0,0);engine_bass_on(1,67,0,0);record(reference);
   FILE *f=fopen(dump,"wb");CHECK(f);CHECK(fwrite(reference,sizeof reference,1,f)==1&&!fclose(f));return 0;}
 for(unsigned owner=0;owner<2;owner++){
  stabilize();watch=&parts[owner]->e;
  actual_rate_oracle(watch);
  CHECK(s1c_front_set(watch,139,0.f));CHECK(s1c_front_set(watch,36,9.f));CHECK(s1c_front_set(watch,35,0.f));
  CHECK(s1c_front_set(watch,40,.7f)&&s1c_front_set(watch,37,1.f));
  CHECK(s1c_front_set(watch,44,.65f)&&s1c_front_set(watch,41,1.f));
  watch->pan_authored=1;oracle_init(watch);active=1;seen=left=right=nonzero=zero_mono=0;maxgain=maxpower=0.;
  CHECK(s1_part_note(parts[owner],60,101));
  static float l[256],r[256];unsigned tail=0;
  for(unsigned frame=0;frame<72;frame++) {
    if(frame==9){CHECK(s1c_front_set(watch,35,1.f));CHECK(s1c_front_set(watch,36,8.5f));}
    if(frame==38){CHECK(s1c_front_set(watch,36,5.f));}
    if(frame==54){
      /* Drop voice ingress, retain real preexisting wet delay/reverb histories. */
      for(unsigned i=0;i<S1C_VOICES;i++)watch->v[i].stage=0;
      CHECK(s1c_front_set(watch,35,.35f));CHECK(s1c_front_set(watch,36,7.f));
    }
    s1_part_render(parts[owner],l,r,256);
    if(frame>=54)for(unsigned i=0;i<256;i++)tail+=fabsf(l[i])+fabsf(r[i])>1.e-5f;
  }
  active=0;CHECK(seen==72*256 && zero_mono>=18*256&&nonzero>10000&&tail>1000&&left>100&&right>100);
  fprintf(stderr,"owner%u frames=%u nonzero=%u L/R=%u/%u wet-tail=%u phase=exact gain max=%g power max=%g\n",
          owner,seen,nonzero,left,right,tail,maxgain,maxpower);
  CHECK(parts[1-owner]->e.pan_authored==0);
 }
 CHECK(!memcmp(&original,&proj.sound,sizeof original)&&writes==store_writes);
 printf("PASS both actual S1 AutoPan receivers, independent original Soundpipe phase and sides, free/bounded clock-rate and wet history: %u checks\n",checks);
}
