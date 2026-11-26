#include "micro.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
enum { EARLY=1088, NT=113+167+193+241 };
typedef struct { float a,b; } B;
typedef struct {
 uint8_t early[2][EARLY];
 int16_t tank[NT];
 B input[2][2], output[2][2];
 float v[4], lp[4], s[2], gains[4], fb, prev;
 uint16_t ew, tw[4], period, dec;
} Room;
_Static_assert(sizeof(Room)<=4096,"4KiB cap all mutable state and history");
static const unsigned lens[4]={113,167,193,241},ofs[4]={0,113,280,473};
static const float trace[2][4]={{2209.f/4,2471.f/4,3260.f/4,3923.f/4},{1945.f/4,2805.f/4,3557.f/4,4148.f/4}};
static float shelf=.3f,pow_scale=1.f,late_gain=.35f,late_in=1.f;
void micro_config(float p,float sh,float g,float in){pow_scale=p;shelf=sh;late_gain=g;late_in=in;}
size_t micro_size(void){return sizeof(Room);}
void *micro_new(void){Room*q=calloc(1,sizeof*q);q->prev=-1;return q;}
void micro_drop(void*q){free(q);}
static uint8_t store8(float x){
 uint32_t v;memcpy(&v,&x,4); unsigned s=v>>31;v&=0x7fffffffu;
 v+=(1u<<20)-1u+((v>>21)&1); unsigned e=v>>23;
 if(e<98u)return (uint8_t)(s<<7);
 if(e>129u)return (uint8_t)((s<<7)|127);
 return (uint8_t)((s<<7)|((e-98)<<2)|((v>>21)&3));
}
static float read8(uint8_t v){
 uint32_t s=(uint32_t)(v&128u)<<24;
 uint32_t f=(v&127u)?s|((((v>>2)&31u)+98u)<<23)|((v&3u)<<21):s;
 float x;memcpy(&x,&f,4);return x;
}
static int16_t store16(float x){
 uint32_t f;memcpy(&f,&x,4);return (uint16_t)((f+0x7fffu+((f>>16)&1u))>>16);
}
static float read16(int16_t v){
 uint32_t f=(uint32_t)(uint16_t)v<<16;float x;memcpy(&x,&f,4);return x;
}
static float filter(B*s,float x){
 /* 2-stage 4.55kHz Butterworth, full clock. */
 static const float c[2][3]={{0.0651116811f,-1.02338008f,0.283826801f},{0.0823950552f,-1.29502812f,0.62460834f}};
 for(unsigned j=0;j<2;j++){const float*p=c[j];float y=p[0]*x+s[j].a;s[j].a=2*p[0]*x-p[1]*y+s[j].b;s[j].b=p[0]*x-p[2]*y;x=y;}
 return x;
}
static float ago(Room*q,unsigned c,float pos){
 unsigned a=(unsigned)pos,u=(q->ew+EARLY-a)%EARLY,v=u?u-1u:EARLY-1u;
 float x=read8(q->early[c][u]),y=read8(q->early[c][v]);return x+(y-x)*(pos-a);
}
void micro_tick(void*ptr,float fb,float L,float R,float*ol,float*orr){
 Room*q=ptr;float l=filter(q->input[0],L),r=filter(q->input[1],R);
 if(!(q->dec++&3u)){
  if(!(q->period++&15u) && q->prev!=fb){
   q->prev=fb;float f=fminf(.999f,fmaxf(0.f,fb));q->fb=f;
   for(unsigned i=0;i<4;i++)q->gains[i]=f>0?powf(f,pow_scale*(float)lens[i]/(11025.f*.069f)):0.f;
  }
  unsigned k=q->ew;q->early[0][k]=store8(l);q->early[1][k]=store8(r);
  float sl=0.f,sr=0.f;
  for(unsigned j=0;j<4;j++){sl+=ago(q,0,trace[0][j]);sr+=ago(q,1,trace[1][j]);}
  float fl=ago(q,0,840.f),fr=ago(q,1,840.f),sum=0.f;
  for(unsigned i=0;i<4;i++){
   float w=read16(q->tank[ofs[i]+q->tw[i]]);
   q->lp[i]+=shelf*(w-q->lp[i]);q->v[i]=q->lp[i]*q->gains[i];sum+=q->v[i];
  }
  for(unsigned i=0;i<4;i++){
   float in=i&1u?fr:fl;
   q->tank[ofs[i]+q->tw[i]]=store16(late_in*q->fb*in*(i>=2?-1.f:1.f)+.5f*sum-q->v[i]);
   if(++q->tw[i]==lens[i])q->tw[i]=0;
  }
  q->s[0]=sl*(.35f*fb)+late_gain*(q->v[0]-q->v[1]-q->v[2]+q->v[3]);
  q->s[1]=sr*(.35f*fb)+late_gain*(q->v[0]+q->v[1]-q->v[2]-q->v[3]);
  if(++q->ew==EARLY)q->ew=0;
 }
 *ol=filter(q->output[0],q->s[0]);*orr=filter(q->output[1],q->s[1]);
}
void micro_run(void*q,const float*x,float*y,const float*fb,unsigned n){
 for(unsigned i=0;i<n;i++){micro_tick(q,*fb++,x[0],x[1],y,y+1);x+=2;y+=2;}
}
