#include <stdio.h>
#include <assert.h>
#include SOURCE
static void put(const void*p,size_t n){assert(fwrite(p,1,n,stdout)==n);}
static void val(s1c*s,int i,float v){s->t[i]=v;if(s1c_smap[i]>=0)s->c[s1c_smap[i]]=v;}
static uint32_t rr=42;
static uint32_t next(void){return rr=rr*1664525u+1013904223u;}
static volatile float sink;
int main(void) {
 const float fs[]={22050,44100,48000,96000};
#ifndef BENCH
 const uint32_t borders[]={0,1,0x7ffffff,0x1fffffffu,0x20000000u,0x20000001u,0x7fffffffu,0x80000000u,0x80000001u,0xffffffffu};
 for(unsigned i=0;i<262144;i++){
  uint32_t ph=i%2?next():borders[(i>>1)%10], step=next();
  float dt=i%7==0?(i&1?0.f:-0.f):(i%7)*(.5f/6.f);
  int k=(i>>3)%4;
  float fr=(i%5)*.25f;
  float y=morph_step(&ph,dt,k,fr,i%3!=0,step);
  put(&ph,4);put(&y,4);
 }
#endif
 for(unsigned test=0;test<48;test++){
   s1c s; s1c_init(&s,fs[test%4],0);
   for(int j=0;j<S1C_VOICES;j++){s.v[j].root=(int8_t)(42+5*j);s.v[j].stage=1;s.v[j].gate=1.f;s.v[j].freq=note_hz(s.v[j].root);s.v[j].vamp=.85f;s.v[j].ph[0]=next();s.v[j].ph[1]=next();s.v[j].ph[2]=next();}
   val(&s,VOL1,.45f);val(&s,VOL2,.53f);val(&s,AA,.015f);val(&s,AD,.1f);val(&s,AS,.55f);val(&s,AR,.08f);
   val(&s,FA,.023f);val(&s,FD,.15f);val(&s,FS,.37f);val(&s,FR,.09f);val(&s,FTYPE,test%3);val(&s,BLE,test&1);
   val(&s,BAL,.41f);val(&s,DETM,1.f);val(&s,I1,.17f);val(&s,I2,.92f);val(&s,SUBV,.3f);val(&s,SUBSQ,(test>>1)&1);
   val(&s,FMV,(test&4)?.21f:0.f);val(&s,FMA,.29f);val(&s,NOISEV,(test&8)?.08f:0.f);val(&s,FMIX,.73f);val(&s,RES,.55f);
   val(&s,MONO,(test&16)!=0);s.monos=449.2f;
   s1c_morph m[2]={morph_params(.97f),morph_params(.9998104f)};
   for(unsigned f=0;f<65536;f++){
      if(f==15098)for(int j=0;j<S1C_VOICES;j++)s.v[j].gate=0;
      if(f==30047)for(int j=0;j<S1C_VOICES;j++)s.v[j].gate=1;
      if(f%127==0){
        val(&s,CUT,100+next()%20000);val(&s,DET2,(float)(int32_t)next()*(.15f/2147483648.f));val(&s,SEMI1,(int)(next()%71)-35);val(&s,SEMI2,-((int)(next()%71)-35));
        val(&s,SUBOCT,next()&1);val(&s,KTRK,(float)(next()%100)*.01f);val(&s,FENV,.37f);
      }
      float x=0.f;
      int n=s.t[MONO]>0?1:S1C_VOICES;
      for(int j=0;j<n;j++) {
         float v=voice_run_kr(&s,&s.v[j],m,.1f,NULL,NULL,NULL
#if S1C_ENVELOPE_SHARE
                              ,NULL
#endif
                             );
#ifndef BENCH
         put(&v,sizeof v);
#endif
         x+=v;
      }
      sink=x;
#ifndef BENCH
      if(f%127==0){put(s.v,sizeof s.v);put(&s.rnd,sizeof s.rnd);}
#endif
   }
 }
}
