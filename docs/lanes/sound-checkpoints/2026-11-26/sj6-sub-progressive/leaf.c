#include <stdint.h>
#include <stddef.h>
#include <limits.h>
/* Host feasibility only; never registered as a shipping provider. */
static uint16_t u16(const uint8_t *p){return (uint16_t)(p[0]|(p[1]<<8));}
static int32_t s16(const uint8_t *p){uint32_t u=u16(p);return u>=32768u ? (int32_t)u-65536 : (int32_t)u;}
/* Header mode 0=raw; 1=constant; 2=2nd-order delta + packed signed words. */
int decode_exact(const uint8_t *p,size_t len,int16_t *out,unsigned n){
    if(!p||!out||n<2||n>256||len<1) return 0;
    if(p[0]==0){
        if (len!=1u+2u*n)return 0;
        for(unsigned j=0;j<n;j++)out[j]=(int16_t)s16(p+1+2*j);
        return 1;
    }
    if(p[0]==1){
        if (len!=3)return 0;
        int16_t v=(int16_t)s16(p+1);
        for(unsigned j=0;j<n;j++)out[j]=v;
        return 1;
    }
    if(p[0]!=2||len<6)return 0;
    unsigned w=p[5];
    if(w>18 || len!=6+((n-2)*w+7)/8)return 0;
    unsigned at=6,bits=0;
    uint32_t word=0,mask=(1u<<w)-1;
    int32_t a=s16(p+1),b=s16(p+3);
    out[0]=(int16_t)a;out[1]=(int16_t)b;
    for(unsigned j=2;j<n;j++){
        while(bits<w){word|=(uint32_t)p[at++]<<bits;bits+=8;}
        uint32_t zz=word&mask; word>>=w;bits-=w;
        int32_t diff=(int32_t)(zz>>1)-(int32_t)((zz&1)?((zz>>1)*2+1):0);
        int32_t x=(2*b-a)+diff;
        if(x<INT16_MIN||x>INT16_MAX)return 0;
        out[j]=(int16_t)x;a=b;b=x;
    }
    return !(word || at!=len);
}
/* Two disjoint decoded slots maintained by producer; no flash or callbacks here. */
typedef struct {const int16_t *row[2];unsigned first[2],count[2];} progressive_window;
static int one(const progressive_window*w,unsigned u,int16_t *v){
 for(unsigned s=0;s<2;s++)if(w->row[s]&&u>=w->first[s]&&u-w->first[s]<w->count[s]){
     *v=w->row[s][u-w->first[s]];return 1;
 }
 return 0;
}
int exact_ready_pair(const progressive_window*w,unsigned u,int16_t*a,int16_t*b){
    int16_t x,y;
    if(!w||!a||!b||u==UINT_MAX||!one(w,u,&x)||!one(w,u+1,&y))return 0;
    *a=x;*b=y;return 1;
}
