/* Uniform signed 6-bit per channel, fixed age tiers. A feasibility probe,
 * not the production time/wet ramp. Each read is random-access and bounded. */
#include <stdint.h>
#include <stddef.h>

#define T 12u
#define GUARD 8u
static const uint32_t stride[T]={1,5,10,30,90,270,810,2430,7290,21870,65610,196830};
static const uint16_t cap[T]={48,2224,28,28,28,28,28,28,28,28,28,56};
static const uint16_t base[T]={0,72,3408,3450,3492,3534,3576,3618,3660,3702,3744,3786};
static const uint8_t ratio[T]={1,5,2,3,3,3,3,3,3,3,3,3};
enum { RING_BYTES=3870 };
typedef struct {
 uint8_t ring[RING_BYTES];
 uint16_t pos[T],fill[T];
 uint32_t end[T],frame;
 float sum[T][2];
 uint8_t phase[T];
} Data;
typedef union { float f; uint32_t u; } fbits;
static unsigned enc(float v)
{
 fbits x={.f=v};
 unsigned m=(x.u&0x7fffffffu)+(1u<<21),e=m>>23;
 if(e<115)return 0;
 unsigned q=e>129?31u:((e-114u)<<1)|(m>>22&1u);
 return ((x.u>>26)&32u)|q;
}
static float dec(unsigned v)
{
 fbits x={.u=v?((v&32u)<<26)|(((v&31u)>>1)+114u)<<23|((v&1u)<<22):0};
 return x.f;
}
static void put(uint8_t *b,unsigned i,unsigned v)
{
 unsigned j=(i/2)*3;
 if(i&1){b[j+1]=(uint8_t)((b[j+1]&15u)|(v<<4));b[j+2]=(uint8_t)(v>>4);}
 else {b[j]=(uint8_t)v;b[j+1]=(uint8_t)((b[j+1]&240u)|(v>>8));}
}
static unsigned get(const uint8_t *b,unsigned i)
{
 unsigned j=(i/2)*3;
 return i&1?((b[j+1]>>4)|((unsigned)b[j+2]<<4)):(b[j]|((b[j+1]&15u)<<8));
}
static void write(Data *d,float a,float b)
{
 uint32_t ts=d->frame;
 for(unsigned t=0;t<T;t++){
  const unsigned at=d->pos[t], nt=t+1u;
  put(d->ring+base[t],at,enc(a)|(enc(b)<<6));
  d->end[t]=ts;
  d->pos[t]=at+1u==cap[t]?0u:at+1u;
  if(d->fill[t]<cap[t])++d->fill[t];
  if(nt>=T || d->fill[t]<cap[t]-GUARD)break;
  unsigned k=at+GUARD+1u;
  if(k>=cap[t])k-=cap[t];
  unsigned sample=get(d->ring+base[t],k);
  d->sum[nt][0]+=dec(sample&63u);d->sum[nt][1]+=dec(sample>>6);
  if(++d->phase[nt]!=ratio[nt])break;
  a=d->sum[nt][0]/(float)ratio[nt];b=d->sum[nt][1]/(float)ratio[nt];
  d->sum[nt][0]=d->sum[nt][1]=0.f;d->phase[nt]=0;
  ts-=(uint32_t)(cap[t]-GUARD-1u)*stride[t]+((uint32_t)(ratio[nt]-1u)*stride[t])/2u;
 }
 ++d->frame;
}
static float first(const Data *d,unsigned t)
{
 return (float)(uint32_t)(d->frame-d->end[t])+(d->fill[t]-1u)*(float)stride[t];
}
static int read_tier(const Data *d,unsigned t,float age,float *l,float *r)
{
 if(!d->fill[t])return 0;
 float v=(age-(float)(uint32_t)(d->frame-d->end[t]))/(float)stride[t];
 if(v<0.f || v>(float)(d->fill[t]-1u))return 0;
 unsigned k=(unsigned)v+1u,current=d->pos[t];
 float f=v-(float)(k-1u);
 unsigned i=current>=k?current-k:current+cap[t]-k;
 unsigned a=get(d->ring+base[t],i);
 unsigned b=k<d->fill[t]?get(d->ring+base[t],i?i-1u:cap[t]-1u):a;
 float al=dec(a&63u),ar=dec(a>>6);
 *l=al+(dec(b&63u)-al)*f;*r=ar+(dec(b>>6)-ar)*f;
 return 1;
}
int seek_age(const Data *d,float age,float *l,float *r)
{
 *l=*r=0.f;
 for(unsigned t=0;t<T;t++){
  if(!read_tier(d,t,age,l,r))continue;
  /* A bounded 4-cell convex adjacent-tier fade, always inside real overlap. */
  float blend=(age-first(d,t))/(4.f*(float)stride[t])+1.f;
  if(t+1u<T && blend>0.f){
   float bL,bR;
   if(read_tier(d,t+1u,age,&bL,&bR)){
     *l+=(bL-*l)*blend;*r+=(bR-*r)*blend;
   }
  }
  return (int)t;
 }
 return -1;
}
void stream_mix(Data *d,size_t n,const float *l,const float *r,float *ol,float *orr,float age,float fb,float wet)
{
 for(size_t i=0;i<n;i++){
  float dl,dr;
  (void)seek_age(d,age,&dl,&dr);
  write(d,l[i]+fb*dl,r[i]+fb*dr);
  ol[i]=wet==0.f?l[i]:l[i]*(1.f-wet)+dl*wet;
  orr[i]=wet==0.f?r[i]:r[i]*(1.f-wet)+dr*wet;
 }
}
uint8_t store_layout[sizeof(Data)];
size_t state_bytes(void){return sizeof(Data);}
