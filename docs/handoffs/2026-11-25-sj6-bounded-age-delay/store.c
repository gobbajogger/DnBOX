/* Uniform signed 6-bit per channel, fixed age tiers. A feasibility probe,
 * not the production time/wet ramp. Each read is random-access and bounded. */
#include <stdint.h>
#include <stddef.h>
#define T 5u
static const uint32_t stride[T]={1,5,512,8192,65536};
static const uint16_t cap[T]={48,2224,96,112,130}; /* all even */
static const uint16_t base[T]={0,72,3408,3552,3720};
enum { RING_BYTES=3915 };
typedef struct {
 uint8_t ring[RING_BYTES];
 uint16_t pos[T],fill[T],phase[T];
 float sum[T][2];
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
 for(unsigned t=0;t<T;t++){
  d->sum[t][0]+=a;d->sum[t][1]+=b;
  if((uint32_t)d->phase[t]+1u<stride[t]){++d->phase[t];continue;}
  d->phase[t]=0;
  float inv=1.f/(float)stride[t];
  put(d->ring+base[t],d->pos[t],enc(d->sum[t][0]*inv)|(enc(d->sum[t][1]*inv)<<6));
  d->sum[t][0]=d->sum[t][1]=0.f;
  if(++d->pos[t]==cap[t])d->pos[t]=0;
  if(d->fill[t]<cap[t])++d->fill[t];
 }
}
static void read_tier(const Data *d,unsigned t,float age,float *l,float *r)
{
 float v=(age-(float)d->phase[t]+.5f*((float)stride[t]-1.f))*(1.f/(float)stride[t]);
 if(v<1.f)v=1.f;
 unsigned k=(unsigned)v, current=d->pos[t];
 float f=v-(float)k;
 if(k>d->fill[t]){*l=*r=0;return;}
 unsigned i=current>=k?current-k:current+cap[t]-k;
 unsigned a=get(d->ring+base[t],i);
 unsigned b=k<d->fill[t]?get(d->ring+base[t],i?i-1u:cap[t]-1u):0;
 float al=dec(a&63u), ar=dec(a>>6);
 *l=al+(dec(b&63u)-al)*f;*r=ar+(dec(b>>6)-ar)*f;
}
unsigned tier(float age)
{
 unsigned t=0;
 while(t<T-1 && age>(float)(cap[t]-2u)*(float)stride[t])++t;
 return t;
}
void seek_age(const Data *d,float age,float *l,float *r)
{ read_tier(d,tier(age),age,l,r); }
void stream_mix(Data *d,size_t n,const float *l,const float *r,float *ol,float *orr,float age,float fb,float wet)
{
 const unsigned t=tier(age);
 for(size_t i=0;i<n;i++){
  float dl,dr;
  read_tier(d,t,age,&dl,&dr);
  write(d,l[i]+fb*dl,r[i]+fb*dr);
  ol[i]=wet==0.f?l[i]:l[i]*(1.f-wet)+dl*wet;
  orr[i]=wet==0.f?r[i]:r[i]*(1.f-wet)+dr*wet;
 }
}
uint8_t store_layout[sizeof(Data)];
size_t state_bytes(void){return sizeof(Data);}
