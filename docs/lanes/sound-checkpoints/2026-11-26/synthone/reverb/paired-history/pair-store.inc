/* Host-only pair-history observation. All original RevSC code outside ring IO is unchanged.
 * Pending preserves the true newly written even sample while its partner is still previous-cycle.
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef ROOM_LOW
#define ROOM_LOW 64
#endif
#ifndef ROOM_HIGH
#define ROOM_HIGH 64
#endif
typedef struct {float even;unsigned pending;} pending_pair;
static unsigned long long room_pairs, room_tail_singletons, room_unequal_words, room_saturated[2], room_rounded_zero[2];
static double room_energy[2], room_error_energy[2], room_peak[2], room_exact_worst, room_exact_worst_relative;
static size_t room_stride(size_t n){
 size_t count=(n+1)/2;
 size_t bytes=(count*(ROOM_LOW+ROOM_HIGH)+7)/8+sizeof(pending_pair);
 return (bytes+7)&~(size_t)7;
}
static unsigned getbits(const unsigned char *p,size_t offset,unsigned n){
 unsigned x=0;
 for(unsigned j=0;j<n;++j)if(p[(offset+j)>>3]&(1u<<((offset+j)&7u)))x|=1u<<j;
 return x;
}
static void putbits(unsigned char *p,size_t offset,unsigned n,unsigned x){
 for(unsigned j=0;j<n;++j){unsigned a=1u<<((offset+j)&7u);
   if(x&(1u<<j))p[(offset+j)>>3]|=(unsigned char)a;
   else p[(offset+j)>>3]&=(unsigned char)(~a);
 }
}
static unsigned rne(float a){unsigned j=(unsigned)a;float f=a-(float)j;return j+(f>.5f || (f==.5f && (j&1u)));}
static void format(unsigned bits,unsigned lane,unsigned *eb,unsigned *mb,int *bias){
 *eb=bits==8?4:bits==12?5:8;*mb=bits==8?3:bits==12?6:7;*bias=bits==8?11:bits==12?24:127;
#if defined(ROOM_H16)
 if(bits==16){*eb=5;*mb=10;*bias=22;}
#endif
#if defined(ROOM_FRONTIER)
 if(!lane&&bits>=11&&bits<=14){*eb=4;*mb=bits-5;*bias=11;}
 if(lane&&bits>=4&&bits<=6){*eb=3;*mb=bits-4;*bias=6;}
#endif
}
static unsigned encode(float x,unsigned bits,unsigned lane){
 unsigned eb,mb;int bias;format(bits,lane,&eb,&mb,&bias);
 unsigned sign=signbit(x)?1u:0u, em=(1u<<eb)-1, mm=1u<<mb;
 float a=fabsf(x); if(a==0.f)return 0;
 int e;float f=frexpf(a,&e);
 int k=e-1+bias;unsigned z;
 if(k<=0){z=rne(ldexpf(a,(int)mb+bias-1));if(!z)++room_rounded_zero[lane];}
 else{
  unsigned m=rne((2.f*f-1.f)*(float)mm);
  if(m==mm){++k;m=0;}
  if((unsigned)k>=em){++room_saturated[lane];k=(int)em-1;m=mm-1;}
  z=((unsigned)k<<mb)|m;
 }
 return (sign<<(eb+mb))|z;
}
static float decode(unsigned x,unsigned bits,unsigned lane){
 unsigned eb,mb;int b;format(bits,lane,&eb,&mb,&b);
 const unsigned cap=1u<<mb;
 unsigned e=(x>>mb)&((1u<<eb)-1),m=x&(cap-1);
 float f=e?ldexpf(1.f+(float)m/cap,(int)e-b):ldexpf((float)m,1-b-(int)mb);
 return x>>(eb+mb)?-f:f;
}
static void pair_convert(void *data,unsigned idx,float v,float w){
 const double vl=v,wr=w,low=(vl+wr)*.5,high=(vl-wr)*.5;
 unsigned char *p=(unsigned char *)data+sizeof(pending_pair);
 size_t off=(size_t)(idx>>1)*(ROOM_LOW+ROOM_HIGH);
 double band[2]={low,high}, rec[2];
 ++room_pairs;
 for(unsigned j=0;j<2;j++){
  unsigned n=j?ROOM_HIGH:ROOM_LOW;
  double a=band[j];room_energy[j]+=a*a;
  if(fabs(a)>room_peak[j])room_peak[j]=fabs(a);
  if(n==64){
   memcpy(p+(off>>3),&a,8);rec[j]=a;
  }else{
   unsigned x=encode((float)a,n,j);putbits(p,off,n,x);rec[j]=decode(x,n,j);
  }
  double e=rec[j]-a;room_error_energy[j]+=e*e;
  off+=n;
 }
 const float aa=(float)(low+high),bb=(float)(low-high);
 const float original[2]={v,w},decoded[2]={aa,bb};
 for(unsigned j=0;j<2;j++){
  if(memcmp(&original[j],&decoded[j],4)){
   ++room_unequal_words;double e=fabs((double)original[j]-decoded[j]);
   if(e>room_exact_worst)room_exact_worst=e;
   double r=e/(fabs(original[j])+1.e-300);if(r>room_exact_worst_relative)room_exact_worst_relative=r;
  }
 }
}
static void room_put(sp_revsc_dl *line,unsigned i,float x){
 pending_pair *h=(pending_pair *)line->buf;
 if(!(i&1u)){
  if(i+1u==(unsigned)line->bufferSize){++room_tail_singletons;pair_convert(line->buf,i,x,0.f);h->pending=0;}
  else{h->even=x;h->pending=i+1u;}
 } else{
  if(h->pending!=i)abort();
  pair_convert(line->buf,i,h->even,x);h->pending=0;
 }
}
static float room_read(const sp_revsc_dl *line,unsigned i){
 const pending_pair *h=(const pending_pair*)line->buf;
 if(!(i&1u)&&h->pending==i+1u)return h->even;
 const unsigned char *p=(const unsigned char *)line->buf+sizeof(pending_pair);
 size_t off=(size_t)(i>>1)*(ROOM_LOW+ROOM_HIGH);
 double v[2];
 for(unsigned j=0;j<2;j++){
  unsigned n=j?ROOM_HIGH:ROOM_LOW;
  if(n==64)memcpy(&v[j],p+(off>>3),8);
  else v[j]=decode(getbits(p,off,n),n,j);
  off+=n;
 }
 return (float)(v[0]+((i&1u)?-v[1]:v[1]));
}
void room_storage_stats(sp_revsc*x){
 unsigned cells=0,history=0;
 for(unsigned j=0;j<8;j++){
  unsigned n=x->delayLines[j].bufferSize;
  cells+=n;history+=room_stride(n);
 }
 fprintf(stderr,"{\"format\":[%d,%d],\"ring_slots\":%u,\"history_bytes_including_pending\":%u,\"state_bytes\":%zu,\"pairs\":%llu,\"tail_singletons\":%llu,\"double_roundtrip_mismatches\":%llu,\"double_roundtrip_max_error\":%.17g,\"double_roundtrip_max_relative_error\":%.17g,\"band_energy\":[%.17g,%.17g],\"band_peak\":[%.17g,%.17g],\"codec_error_energy\":[%.17g,%.17g],\"saturations\":[%llu,%llu],\"nonzero_to_zero\":[%llu,%llu]}\n",ROOM_LOW,ROOM_HIGH,cells,history,sizeof(*x),room_pairs,room_tail_singletons,room_unequal_words,room_exact_worst,room_exact_worst_relative,room_energy[0],room_energy[1],room_peak[0],room_peak[1],room_error_energy[0],room_error_energy[1],room_saturated[0],room_saturated[1],room_rounded_zero[0],room_rounded_zero[1]);
}
