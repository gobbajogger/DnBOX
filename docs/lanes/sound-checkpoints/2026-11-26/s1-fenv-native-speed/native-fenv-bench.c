#include <stddef.h>
#include <stdint.h>
#define X0X_IMA_TABLES_DEFINE 1
#include "dsp/ima_adpcm.h"
#include "dsp/s1c/s1c.h"
#include "dsp/s1c/s1c_front.h"
#include "fm1_time.h"
void *memset(void *s,int v,size_t n){unsigned char *p=s;while(n--)*p++=(unsigned char)v;return s;}
void *memcpy(void *s,const void *v,size_t n){unsigned char *p=s;const unsigned char*q=v;while(n--)*p++=*q++;return s;}
struct r { uint32_t ticks, hash, active; };
static volatile uint32_t *const status=(void*)0x01c10000u;
static volatile struct r *const out=(void*)0x01c10008u;
static s1c *const k=(void*)0x01c1a000u;
static void reset(unsigned f,unsigned route,unsigned voices) {
 s1c_init(k,44100,0);
 s1c_attach(k,(void*)0x01c20000u,(void*)0x01c22000u,(void*)(0x01c20000u+S1C_RVB_BYTES),S1C_ARENA_BYTES-S1C_RVB_BYTES);
 k->rv.fit[0]=.5f;k->rv.fit[1]=1.5f;k->rv.fit[2]=1.f;k->dfmt=0;
 for(unsigned i=0;i<150;i++)s1c_front_set(k,i,s1c_front_init(i));
 s1c_front_set(k,14,.73f); s1c_front_set(k,46,.61f); s1c_front_set(k,19,.54f);
 s1c_front_set(k,56,(float)route); s1c_front_set(k,115,(float)f);
 for(unsigned i=0;i<voices;i++)s1c_start(k,43+4*i,90+i);
}
void bench_start(){
 fm1_time_init(); unsigned row=0; *status=0;status[1]=0;
 for(unsigned f=0;f<3;f++)for(unsigned route=0;route<4;route++)for(unsigned v=1;v<=4;v+=3){
  reset(f,route,v);float l[64],r[64]; uint32_t total=0,h=2166136261u;
  for(unsigned j=0;j<80;j++){
   if(j==30)s1c_front_set(k,56,(float)((route+1)%4));
   if(j==40)s1c_front_set(k,56,(float)route);
   if(j==64)for(unsigned i=0;i<v;i++)s1c_stop(k,43+4*i);
   const uint32_t t=fm1_ticks();s1c_render(k,l,r,64);total+=fm1_ticks()-t;
   for(unsigned i=0;i<64;i++){
    union {float f;uint32_t u;} a={l[i]},b={r[i]};
    if((a.u&0x7f800000u)==0x7f800000u||(b.u&0x7f800000u)==0x7f800000u)status[1]++;
    h=(h^a.u)*16777619u;h=(h^b.u)*16777619u;
   }
  }
  out[row++]=(struct r){total,h,v};
 }
 *status=0x46454e56;
}
