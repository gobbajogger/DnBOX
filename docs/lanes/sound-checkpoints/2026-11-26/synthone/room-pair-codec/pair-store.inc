/* Experimental full-rate RevSC history IO only. Separate block exponents,
 * exact staging for the currently written region, random reads (no decode state). */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef ROOM_LOW
#define ROOM_LOW 11
#endif
#ifndef ROOM_HIGH
#define ROOM_HIGH 6
#endif
#ifndef BLOCK
#define BLOCK 16
#endif
typedef struct {unsigned block, used; float sample[BLOCK*2];} staging;
enum { width = ROOM_LOW+ROOM_HIGH, payload=(BLOCK*width+7)/8, packet=payload+2 };
static size_t room_stride(size_t n){return (sizeof(staging)+((n+2*BLOCK-1)/(2*BLOCK))*packet+7)&~(size_t)7;}
/* Bounded little-endian extraction: each coefficient spans at most 3 bytes. */
static unsigned getbits(const unsigned char *p,unsigned offset,unsigned n){
 unsigned k=offset&7u;const unsigned char *q=p+(offset>>3);
 unsigned w=q[0];if(k+n>8)w|=(unsigned)q[1]<<8;
 if(k+n>16)w|=(unsigned)q[2]<<16;
 return (w>>k)&((1u<<n)-1u);
}
static void putbits(unsigned char *p,unsigned offset,unsigned n,unsigned x){
 unsigned k=offset&7u;unsigned char *q=p+(offset>>3);x<<=k;
 q[0]|=(unsigned char)x;if(k+n>8)q[1]|=(unsigned char)(x>>8);
 if(k+n>16)q[2]|=(unsigned char)(x>>16);
}
static unsigned long long clips[2],zero[2],commits;
/* Scale ceil(peak/limit) to one of four steps per exponent; power-of-2
 * envelopes may waste nearly one coefficient bit on every packet. */
static float room_step(unsigned code){
 return code?ldexpf((float)(4u+(code&3u)),(int)(code>>2)-50):0.f;
}
static unsigned room_step_ceiling(float x,unsigned j){
 int e;float m=frexpf(x,&e);int n=e-1+48;
 if(n<0)return 1;
 unsigned f=(unsigned)ceilf(8.f*m-4.f);
 if(f>=4){++n;f=0;}
 if(n>=64){++clips[j];return 255;}
 unsigned code=(unsigned)n*4+f;
 return code?code:1;
}
static float coefficient(const unsigned char*p,unsigned i,unsigned j){
 if(!p[j])return 0.f;
 unsigned bits=j?ROOM_HIGH:ROOM_LOW;
 int u=(int)getbits(p+2,i*width+(j?ROOM_LOW:0),bits),m=1<<(bits-1);
 return (float)((u^m)-m)*room_step(p[j]);
}
static float prediction(const unsigned char*p,unsigned i,unsigned n){
 float a=coefficient(p,i,0);
#if defined(ROOM_CUBIC_PREDICT)
 if(i&&i+2<n)return 0.5625f*(a+coefficient(p,i+1,0))-0.0625f*(coefficient(p,i-1,0)+coefficient(p,i+2,0));
#endif
 if(i+1<n)return 0.5f*(a+coefficient(p,i+1,0));
 if(i)return 1.5f*a-0.5f*coefficient(p,i-1,0);
 return a;
}
static float band(const staging*a,const unsigned char*p,unsigned i,unsigned j,unsigned n){
 return j?a->sample[i*2+1]-prediction(p,i,n):a->sample[i*2];
}
static void room_commit(staging *a, unsigned char *p,unsigned n){
 memset(p,0,packet);++commits;
 for(unsigned j=0;j<2;++j){
   const unsigned bits=j?ROOM_HIGH:ROOM_LOW;const int lim=(1<<(bits-1))-1;
   float peak=0;for(unsigned i=0;i<n;++i){float v=fabsf(band(a,p,i,j,n));if(v>peak)peak=v;}
   if(!peak)continue;
   p[j]=(unsigned char)room_step_ceiling(peak/(float)lim,j);
   float scale=room_step(p[j]);
   for(unsigned i=0;i<n;++i){
     float v=band(a,p,i,j,n);int q=(int)nearbyintf(v/scale);
     if(!q&&v)++zero[j];
     if(q>lim||q<-lim){++clips[j];q=q>0?lim:-lim;}
     putbits(p+2,i*width+(j?ROOM_LOW:0),bits,(unsigned)q&((1u<<bits)-1));
   }
 }
}
static void room_put(sp_revsc_dl *line,unsigned i,float x){
 staging *a=(staging*)line->buf;
 const unsigned b=i/(2*BLOCK),k=i%(2*BLOCK);
 if(k==0){a->block=b;a->used=0;}
 if(a->block!=b||a->used!=k)abort();
 a->sample[k]=x;++a->used;
 if(k==2*BLOCK-1||i+1u==(unsigned)line->bufferSize){
   if(a->used&1u)a->sample[a->used]=0;
   room_commit(a,(unsigned char*)(a+1)+b*packet,(a->used+1)/2);
   a->used=0;
 }
}
static float room_read(const sp_revsc_dl*line,unsigned i){
 const staging*a=(const staging*)line->buf;const unsigned b=i/(2*BLOCK),k=i%(2*BLOCK);
 if(a->block==b&&k<a->used)return a->sample[k];
 const unsigned char*p=(const unsigned char*)(a+1)+b*packet;
 unsigned rest=line->bufferSize-b*2*BLOCK,n=(rest+1)/2;if(n>BLOCK)n=BLOCK;
 return k&1u?prediction(p,k/2,n)+coefficient(p,k/2,1):coefficient(p,k/2,0);
}
void room_storage_stats(sp_revsc*x){
 unsigned cells=0,history=0;
 for(unsigned j=0;j<8;++j){unsigned n=x->delayLines[j].bufferSize;cells+=n;history+=room_stride(n);}
 fprintf(stderr,"{\"format\":[%d,%d],\"block\":%d,\"ring_slots\":%u,\"history_bytes_including_pending\":%u,\"state_bytes\":%zu,\"commits\":%llu,\"saturations\":[%llu,%llu],\"nonzero_to_zero\":[%llu,%llu]}\n",ROOM_LOW,ROOM_HIGH,BLOCK,cells,history,sizeof(*x),commits,clips[0],clips[1],zero[0],zero[1]);
}
