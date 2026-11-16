// SPDX-License-Identifier: GPL-3.0-only
// Isolated compact v1 word reader. No runtime/product admission or asset placement.
#include "compact.h"
#define S1C_TAG UINT32_C(0x53314331)
#define S1C_MAX_BYTES ((size_t)4194304)
typedef struct {const unsigned char *b;size_t n;uint32_t blocks,data,patch;} view;
typedef struct {uint32_t mode,flags,n,first,off,count;} rowinfo;
static int extent(const void*p,size_t n){return p&&n&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int range(size_t p,size_t n,size_t total){return p<=total&&n<=total-p;}
static int overlap(const void*a,size_t an,const void*b,size_t bn){
 if(!extent(a,an)||!extent(b,bn))return 1;
 return (uintptr_t)a<(uintptr_t)b+bn&&(uintptr_t)b<(uintptr_t)a+an;
}
static uint32_t u32(const unsigned char*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static unsigned u16(const unsigned char*p){return (unsigned)p[0]|((unsigned)p[1]<<8);}
static int word_valid(uint32_t w){return (w&UINT32_C(0x7fffffff))<=UINT32_C(0x40000000);}
static int header(const unsigned char*b,size_t n,view*v){
 if(n<64||n>S1C_MAX_BYTES||!extent(b,n))return 0;
 const uint32_t blocks=u32(b+40),data=u32(b+44),patch=u32(b+48);
 if(u32(b)!=UINT32_C(0x314c3153)||u32(b+4)!=1||u32(b+8)!=50||u32(b+12)!=52||u32(b+16)!=4097||u32(b+20)!=64||
    u32(b+24)!=64||u32(b+28)!=1264||u32(b+32)!=1316||u32(b+36)!=1368||blocks>3200||
    data!=1368+4*(blocks+1)||!range(1368,4*(size_t)(blocks+1),n)||patch<data||patch>n||u32(b+52)!=n||u32(b+56)!=24||u32(b+60))return 0;
 const uint32_t end=u32(b+1368+4*blocks);
 if(u32(b+1368)!=data||end>patch||end>n||((end+3U)&~3U)!=patch)return 0;
 v->b=b;v->n=n;v->blocks=blocks;v->data=data;v->patch=patch;return 1;
}
static int getrow(const view*v,unsigned r,rowinfo*q){
 if(r>=50)return 0;
 const unsigned char*p=v->b+64+24*r;
 q->mode=u32(p);q->flags=u32(p+4);q->n=u32(p+8);q->first=u32(p+12);q->off=u32(p+16);q->count=u32(p+20);
 if(q->mode>3||(q->flags>>14))return 0;
 if(q->mode==3)return !q->flags&&q->n==4096&&q->off>=1&&q->off<=4096&&!q->count&&q->first>=v->patch&&range(q->first,6*(size_t)q->off,v->n);
 const unsigned want=q->mode==0?4096:q->mode==1?2049:1025,nb=(want+63)/64;
 if(q->n!=want||(q->mode==0&&q->flags)||(q->mode==1&&(q->flags&(1U<<13)))||q->first>v->blocks||nb>v->blocks-q->first||
    q->count>4096||q->off<v->patch||!range(q->off,6*(size_t)q->count,v->n))return 0;
 return 1;
}
static int block(const view*v,const rowinfo*q,unsigned j,size_t*p,unsigned*mode,unsigned*width,unsigned*anchors){
 const unsigned nb=(q->n+63)/64;if(j>=nb)return 0;
 const uint32_t index=q->first+j;
 if(index>=v->blocks)return 0;
 const uint32_t start=u32(v->b+1368+4*index),end=u32(v->b+1368+4*(index+1));
 if(start<v->data||end<=start||end>v->patch||end-start<2)return 0;
 const unsigned m=v->b[start],w=v->b[start+1],a=m==0?0:m==1?1:m==3?2:m==4?3:99;
 if(a==99||w>32||(m==0&&w!=32))return 0;
 const unsigned remain=q->n-j*64,n=remain<64?remain:64;
 const size_t size=2+4*a+(m==0?4*(size_t)n:((size_t)n*w+7)/8);
 if(end-start!=size||!range(start,size,v->n))return 0;
 *p=start;*mode=m;*width=w;*anchors=a;return 1;
}
static int physical_word_row(const view*v,const rowinfo*q,unsigned sample,uint32_t*out){
 if(sample>=4097)return 0;
 if(sample==4096){*out=0;return 1;}
 if(q->mode==3){
 unsigned lo=0,hi=q->off;
 while(lo<hi){const unsigned mid=(lo+hi)/2;if(sample<u16(v->b+q->first+6*mid))hi=mid;else lo=mid+1;}
 if(lo>=q->off)return 0;*out=u32(v->b+q->first+6*lo+2);return 1;
 }
 unsigned lo=0,hi=q->count;
 while(lo<hi){const unsigned mid=(lo+hi)/2;if(u16(v->b+q->off+6*mid)<sample)lo=mid+1;else hi=mid;}
 if(lo<q->count&&u16(v->b+q->off+6*lo)==sample){*out=u32(v->b+q->off+6*lo+2);return 1;}
 const unsigned j=(sample+4096-(q->flags&4095))&4095,a=(q->flags>>12)&1,b=(q->flags>>13)&1;
 unsigned rep=j,sign=0;
 if(q->mode){
 const unsigned r=(0U-j)&4095;if(r<rep){rep=r;sign=a;}
 if(q->mode==2){const unsigned t=(j+2048)&4095,s=(2048U-j)&4095;if(t<rep){rep=t;sign=b;}if(s<rep){rep=s;sign=a^b;}}
 }
 if(rep>=q->n)return 0;
 size_t p;unsigned mode,width,anchors;if(!block(v,q,rep/64,&p,&mode,&width,&anchors))return 0;
 const unsigned i=rep%64;uint32_t value;
 if(mode==0)value=u32(v->b+p+2+4*i);
 else{
 uint32_t code=0;const size_t packed=p+2+4*anchors;
 if(width){
 const unsigned bit=i*width,shift=bit&7U,bytes=(shift+width+7U)/8U;
 const unsigned char*bits=v->b+packed+bit/8U;
 code=bits[0];
 if(bytes>1)code|=(uint32_t)bits[1]<<8;
 if(bytes>2)code|=(uint32_t)bits[2]<<16;
 if(bytes>3)code|=(uint32_t)bits[3]<<24;
 code>>=shift;
 if(bytes>4)code|=(uint32_t)bits[4]<<(32U-shift);
 if(width<32)code&=(UINT32_C(1)<<width)-1U;
 }
 const uint32_t base=u32(v->b+p+2);
 if(mode==1)value=base+code;
 else{
 const uint32_t residual=(code>>1)^(0U-(code&1U)),step=u32(v->b+p+6),curve=mode==4?u32(v->b+p+10):0;
 const uint32_t triangle=i?i*(i-1U)/2U:0;value=base+step*i+curve*triangle+residual;
 }
 }
 *out=value^(sign?UINT32_C(0x80000000):0);return 1;
}
static int physical_word(const view*v,unsigned row,unsigned sample,uint32_t*out){
 rowinfo q;if(!getrow(v,row,&q))return 0;return physical_word_row(v,&q,sample,out);
}
static int validate(const view*v){
 for(unsigned r=0;r<52;r++){const unsigned expected=r<48?r:r==48?44:r==49?45:r-2;if(v->b[1264+r]!=expected)return 0;}
 uint32_t previous=0;
 for(unsigned i=0;i<13;i++){const uint32_t w=u32(v->b+1316+4*i),abs=w&UINT32_C(0x7fffffff);if(abs>=UINT32_C(0x7f800000)||((w>>31)&&abs)||(i&&abs<=previous))return 0;previous=abs;}
 uint32_t nextblock=0;size_t nextpatch=v->patch;
 for(unsigned r=0;r<50;r++){
 rowinfo q;if(!getrow(v,r,&q))return 0;
 if(q.mode==3){
 if(q.first!=nextpatch)return 0;unsigned prev=0;
 for(unsigned i=0;i<q.off;i++){const unsigned end=u16(v->b+q.first+6*i);if(end<=prev||end>4096)return 0;prev=end;}
 if(prev!=4096)return 0;nextpatch+=6*(size_t)q.off;
 }else{
 if(q.first!=nextblock||q.off!=nextpatch)return 0;
 const unsigned nb=(q.n+63)/64;
 for(unsigned i=0;i<nb;i++){size_t p;unsigned m,w,a;if(!block(v,&q,i,&p,&m,&w,&a))return 0;}
 unsigned prev=0;
 for(unsigned i=0;i<q.count;i++){const unsigned s=u16(v->b+q.off+6*i);if(s>=4096||(i&&s<=prev))return 0;prev=s;}
 nextblock+=nb;nextpatch+=6*(size_t)q.count;
 }
 }
 if(nextblock!=v->blocks||nextpatch>v->n||((nextpatch+3)&~(size_t)3)!=v->n)return 0;
 for(size_t i=nextpatch;i<v->n;i++)if(v->b[i])return 0;
 for(unsigned r=0;r<50;r++)for(unsigned s=0;s<4097;s++){uint32_t w;if(!physical_word(v,r,s,&w)||!word_valid(w)||(s==4096&&w))return 0;}
 return 1;
}
int s1c_binding_valid(const s1c_binding*b){
 if(!extent(b,sizeof *b)||((uintptr_t)b%__alignof__(s1c_binding))||b->format_tag!=S1C_TAG)return 0;
 view v;return header(b->bytes,b->byte_count,&v);
}
int s1c_bind(s1c_binding*out,const void*data,size_t n){
 if(!extent(out,sizeof *out)||((uintptr_t)out%__alignof__(s1c_binding))||!extent(data,n)||overlap(out,sizeof *out,data,n))return 0;
 view v;if(!header((const unsigned char*)data,n,&v)||!validate(&v))return 0;
 const s1c_binding candidate={(const unsigned char*)data,n,S1C_TAG};*out=candidate;return 1;
}
int s1c_read(const s1c_binding*b,unsigned row,unsigned sample,uint32_t*out){
 if(row>=52||sample>=4097||!extent(b,sizeof *b)||((uintptr_t)b%__alignof__(s1c_binding))||b->format_tag!=S1C_TAG)return 0;
 view v;if(!header(b->bytes,b->byte_count,&v))return 0;
 if(!extent(out,sizeof *out)||((uintptr_t)out%__alignof__(uint32_t))||
    overlap(out,sizeof *out,b,sizeof *b)||overlap(out,sizeof *out,b->bytes,b->byte_count))return 0;
 const unsigned physical=row<48?row:row==48?44:row==49?45:row-2;uint32_t w;
 if(!physical_word(&v,physical,sample,&w)||!word_valid(w))return 0;*out=w;return 1;
}

int s1c_read_quad(const s1c_binding*b,unsigned row0,unsigned row1,unsigned sample0,unsigned sample1,uint32_t*out){
 if(row0>=52||row1>=52||sample0>=4097||sample1>=4097||!extent(b,sizeof *b)||
    ((uintptr_t)b%__alignof__(s1c_binding))||b->format_tag!=S1C_TAG)return 0;
 view v;if(!header(b->bytes,b->byte_count,&v))return 0;
 if(!extent(out,4*sizeof *out)||((uintptr_t)out%__alignof__(uint32_t))||
    overlap(out,4*sizeof *out,b,sizeof *b)||overlap(out,4*sizeof *out,b->bytes,b->byte_count))return 0;
 const unsigned physical0=row0<48?row0:row0==48?44:row0==49?45:row0-2;
 const unsigned physical1=row1<48?row1:row1==48?44:row1==49?45:row1-2;
 rowinfo q0,q1;if(!getrow(&v,physical0,&q0)||!getrow(&v,physical1,&q1))return 0;
 uint32_t w[4];
 if(!physical_word_row(&v,&q0,sample0,w)||!physical_word_row(&v,&q1,sample0,w+1)||
    !physical_word_row(&v,&q0,sample1,w+2)||!physical_word_row(&v,&q1,sample1,w+3))return 0;
 for(unsigned i=0;i<4;i++)if(!word_valid(w[i]))return 0;
 for(unsigned i=0;i<4;i++)out[i]=w[i];return 1;
}
