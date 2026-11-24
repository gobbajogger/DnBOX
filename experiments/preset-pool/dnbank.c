/* SPDX-License-Identifier: GPL-3.0-only */
/* DNL2 experimental exact-word bank, replacing DNB2 reader. */
#include "dnbank.h"
#define HEADER 46u
#define STEP 32u
static uint8_t rd(const dnbank *b,unsigned o){return b->pack[b->sector_map?((unsigned)b->sector_map[o>>12]<<12)+(o&4095u):o];}
static unsigned le16(const dnbank *b,unsigned o){return rd(b,o)|(unsigned)rd(b,o+1)<<8;}
static uint32_t le32(const dnbank *b,unsigned o){return le16(b,o)|(uint32_t)le16(b,o+2)<<16;}
static uint32_t crc(const dnbank *b,unsigned n){uint32_t x=~0u;for(unsigned o=0;o<n;o++){if(o>=8&&o<12)continue;x^=rd(b,o);for(unsigned k=0;k<8;k++)x=(x>>1)^((0u-(x&1u))&0xedb88320u);}return ~x;}
static uint32_t get(const dnbank *b,unsigned *at,unsigned end,unsigned width){uint32_t v=0;unsigned shift=0;if(width>32||*at>end||width>end-*at){*at=end+1;return 0;}while(width){unsigned k=8-(*at&7u);if(k>width)k=width;v|=(uint32_t)((rd(b,*at>>3)>>(*at&7u))&((1u<<k)-1u))<<shift;*at+=k;shift+=k;width-=k;}return v;}
static unsigned minbits(unsigned n){unsigned k=0;while(n){k++;n>>=1;}return k;}
static unsigned block_start(const dnbank *b,unsigned i){unsigned base=b->off[1]+2u*((b->ng+STEP-1u)/STEP);return base+le16(b,b->off[1]+2u*i);}
static unsigned index_record(const dnbank *b,unsigned i){return b->off[5]+2u*b->programs+le16(b,b->off[5]+2u*i);}
static unsigned index_layout(const dnbank *b,unsigned i){return b->off[4]+2u*b->nl+le16(b,b->off[4]+2u*i);}
static int global_word(const dnbank *b,unsigned index,uint32_t *out){if(index>=b->ng)return 0;unsigned i=index/STEP,a=block_start(b,i),n=(b->ng+STEP-1u)/STEP,end=i+1<n?block_start(b,i+1):b->off[2];if(a>end||end-a<5||end>b->off[2])return 0;unsigned k=rd(b,a);if(k>32)return 0;uint32_t w=le32(b,a+1);unsigned at=(a+5u)*8u;for(unsigned j=index%STEP;j;j--){uint32_t d=get(b,&at,end*8u,k);if(!d||d>~w)return 0;w+=d;}if(at>end*8u||(w&0x7f800000u)==0x7f800000u)return 0;*out=w;return 1;}
static int open_core(dnbank *b,size_t length){
 static const uint8_t max[]={22,18,150,220};
 if(le32(b,0)!=0x324c4e44u||le32(b,4)!=length||crc(b,(unsigned)length)!=le32(b,8)||rd(b,13)!=1||rd(b,12)>3)return 0;
 b->engine=rd(b,12);b->programs=(uint16_t)le16(b,24);b->ng=(uint16_t)le16(b,26);b->nf=(uint16_t)le16(b,28);b->nl=(uint16_t)le16(b,30);
 if(!b->programs||b->programs>1024||!b->ng||!b->nf||b->nf>max[b->engine]||!b->nl||b->nl>256)return 0;b->gb=(uint8_t)minbits(b->ng-1u);
 for(unsigned i=0;i<7;i++){b->off[i]=(uint16_t)le16(b,32+2*i);if(b->off[i]>length||(i&&b->off[i]<b->off[i-1]))return 0;}
 const uint16_t *o=b->off;unsigned groups=(b->ng+STEP-1u)/STEP;
 if(o[0]!=HEADER||o[1]-o[0]!=le16(b,14)||o[2]-o[1]<2u*groups||o[3]-o[2]!=6u*b->nf||((o[4]-o[3])&1u)||o[5]-o[4]<2u*b->nl||o[6]-o[5]<2u*b->programs||o[6]!=length)return 0;
 if(le16(b,o[1])||le16(b,o[4])||le16(b,o[5]))return 0;
 /* Validate every table extent and every ordered global word, including the tail. */
 uint32_t prev=0;for(unsigned i=0;i<groups;i++){unsigned a=block_start(b,i),end=i+1<groups?block_start(b,i+1):o[2],n=b->ng-i*STEP;if(n>STEP)n=STEP;if(a>end||end-a<5||end>o[2])return 0;unsigned k=rd(b,a);if(k>32||end-a!=5u+((n-1u)*k+7u)/8u)return 0;uint32_t w=le32(b,a+1);unsigned at=(a+5u)*8u;for(unsigned j=0;j<n;j++){if(j){uint32_t d=get(b,&at,end*8u,k);if(!d||d>~w)return 0;w+=d;}if((i||j)&&w<=prev)return 0;if((w&0x7f800000u)==0x7f800000u)return 0;prev=w;}}
 for(unsigned i=0;i<b->nf;i++){unsigned f=o[2]+6*i,k=rd(b,f+1),n=le16(b,f+2),d=le16(b,f+4);if(rd(b,f)>=max[b->engine]||(i&&rd(b,f)<=rd(b,f-6))||k>8||n>(1u<<k)||d+n>(o[4]-o[3])/2u)return 0;for(unsigned j=0;j<n;j++)if(le16(b,o[3]+2*(d+j))>=b->ng)return 0;}
 for(unsigned i=0;i<b->nl;i++){unsigned a=index_layout(b,i),end=i+1<b->nl?index_layout(b,i+1):o[5];if(a>=end||end>o[5]||!rd(b,a)||rd(b,a)>220||end-a!=1u+rd(b,a))return 0;for(unsigned j=a+1;j<end;j++)if(rd(b,j)>=b->nf)return 0;}
 for(unsigned i=0;i<b->programs;i++){unsigned a=index_record(b,i),end=i+1<b->programs?index_record(b,i+1):o[6];if(a>end||end>o[6]||end-a<11||rd(b,a+8)>=b->nl||!rd(b,a+9)||rd(b,a+9)>47||rd(b,a+9)>end-a-10)return 0;}
 return 1;
}
int dnbank_open_mapped(dnbank *b,const uint8_t *data,size_t available,const uint8_t *map,unsigned count,size_t length){if(!b)return 0;b->pack=0;if(!data||length<HEADER||length>65535u)return 0;if(map){if(available%4096u||available/4096u>256||count!=(length+4095u)/4096u)return 0;for(unsigned i=0;i<count;i++){if(map[i]>=available/4096u)return 0;for(unsigned j=0;j<i;j++)if(map[i]==map[j])return 0;}}else if(length>available)return 0;b->pack=data;b->sector_map=map;if(open_core(b,length))return 1;b->pack=0;return 0;}
int dnbank_open(dnbank *b,const uint8_t *p,size_t n){return dnbank_open_mapped(b,p,n,0,0,n);}
int dnbank_preset_id(const dnbank *b,unsigned p,uint8_t id[8]){if(!b||!b->pack||!id||p>=b->programs)return 0;unsigned a=index_record(b,p);for(unsigned i=0;i<8;i++)id[i]=rd(b,a+i);return 1;}
int dnbank_read(const dnbank *b,unsigned p,char name[48],dnbank_emit emit,void *ctx){
 if(!b||!b->pack||p>=b->programs||!name)return -1;const uint16_t *o=b->off;unsigned a=index_record(b,p),end=p+1<b->programs?index_record(b,p+1):o[6];unsigned li=rd(b,a+8),n=rd(b,a+9);a+=10;for(unsigned i=0;i<n;i++){name[i]=(char)rd(b,a++);if(!name[i])return -1;}name[n]=0;
 unsigned lp=index_layout(b,li),cnt=rd(b,lp++),at=a*8u;end*=8u;
 for(unsigned i=0;i<cnt;i++){unsigned f=o[2]+6*rd(b,lp+i),k=rd(b,f+1),n=le16(b,f+2),v=get(b,&at,end,k);if(v>n)return -1;v=v==n?get(b,&at,end,b->gb):le16(b,o[3]+2*(le16(b,f+4)+v));uint32_t w;if(at>end||!global_word(b,v,&w))return -1;if(emit)emit(ctx,rd(b,f),w);}
 return at<=end&&end-at<8u?(int)cnt:-1;
}
int dnbank_validate(const dnbank *b){if(!b||!b->pack)return 0;char name[48];uint8_t a[8],c[8];for(unsigned i=0;i<b->programs;i++){if(dnbank_read(b,i,name,0,0)<=0||!dnbank_preset_id(b,i,a))return 0;for(unsigned j=0;j<i;j++){dnbank_preset_id(b,j,c);unsigned k=0;while(k<8&&a[k]==c[k])k++;if(k==8)return 0;}}return 1;}
