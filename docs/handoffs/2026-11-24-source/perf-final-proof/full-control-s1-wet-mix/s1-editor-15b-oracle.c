/* Host-only feasibility: no protocol or format assignment. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
enum { BYTES=15, RAD=129, PAIR=RAD*RAD, RATE=148, LFO=RATE*RATE*5*5*3 };
typedef struct { uint32_t d[14]; } fields;
static const uint32_t limits[14]={128,128,128,128,128,128,128,128,2,2,3,LFO-1,128,128};
static const uint8_t bits[]={15,15,15,15,2,2,2,21,15};
static void put(uint8_t *b,unsigned *n,unsigned w,uint32_t v)
{ for (unsigned i=0;i<w;i++,(*n)++) b[*n/8]|=(uint8_t)((v>>i&1u)<<(*n&7)); }
static uint32_t get(const uint8_t *b,unsigned *n,unsigned w)
{ uint32_t v=0; for(unsigned i=0;i<w;i++,(*n)++) v|=(uint32_t)(b[*n/8]>>(*n&7)&1u)<<i; return v; }
static int valid(const fields *s)
{ uint32_t any=0; for(unsigned i=0;i<14;i++){if(s->d[i]>limits[i])return 0;any|=s->d[i];}return any!=0; }
static int encode(uint8_t *out,size_t len,const fields *s)
{
    if(!out||!s||len!=BYTES||!valid(s))return 0;
    const uint32_t *d=s->d, v[]={RAD*d[0]+d[1],RAD*d[2]+d[3],RAD*d[4]+d[5],
        RAD*d[6]+d[7],d[8],d[9],d[10],d[11],RAD*d[12]+d[13]};
    uint8_t b[BYTES]={0}; unsigned off=16;
    for(unsigned i=0;i<sizeof bits;i++)put(b,&off,bits[i],v[i]);
    memcpy(out,b,sizeof b); return 1;
}
static int decode(fields *out,const uint8_t *p,size_t len)
{
    if(!out||!p||len!=BYTES||p[0]||p[1]||(p[14]&0xc0))return 0;
    uint32_t v[9]; unsigned off=16;
    for(unsigned i=0;i<sizeof bits;i++)v[i]=get(p,&off,bits[i]);
    fields next={{0}};
    for(unsigned i=0;i<4;i++){
        if(v[i]>=PAIR)return 0;
        next.d[2*i]=v[i]/RAD; next.d[2*i+1]=v[i]%RAD;
    }
    if(v[8]>=PAIR)return 0;
    next.d[8]=v[4]; next.d[9]=v[5]; next.d[10]=v[6]; next.d[11]=v[7];
    next.d[12]=v[8]/RAD; next.d[13]=v[8]%RAD;
    if(!valid(&next))return 0;
    *out=next; return 1;
}
static void roundtrip(const fields *s)
{
    uint8_t b[BYTES];
    fields next;
    assert(encode(b,sizeof b,s) && !b[0] && !b[1] && !(b[14]&0xc0));
    assert(decode(&next,b,sizeof b) && !memcmp(s->d,next.d,sizeof next.d));
}
static void deny(unsigned at,unsigned len,uint32_t value)
{
    const fields sent={{1,2,3,4,5,6,7,8,1,1,1,2,3,4}};
    fields out=sent;
    uint8_t b[BYTES]={0}; unsigned first=16;
    put(b,&first,15,RAD); put(b,&at,len,value);
    assert(!decode(&out,b,sizeof b));
    assert(!memcmp(out.d,sent.d,sizeof sent.d));
}
int main(void)
{
    fields max; memcpy(max.d,limits,sizeof limits);
    const uint8_t golden[]={0,0,0,0x41,0x80,0x20,0x40,0x10,0x20,0xa8,0xbf,0x44,0x64,0x80,0x20};
    uint8_t b[BYTES];
    assert(encode(b,sizeof b,&max) && !memcmp(b,golden,sizeof b));
    assert(!decode(&max,b,sizeof b-1));
    for(unsigned i=0;i<5;i++)for(unsigned a=0;a<RAD;a++)for(unsigned c=0;c<RAD;c++) {
        fields next=max; unsigned at=i<4?2*i:12;
        next.d[at]=a; next.d[at+1]=c; roundtrip(&next);
    }
    for(unsigned a=0;a<3;a++)for(unsigned c=0;c<3;c++)for(unsigned d=0;d<4;d++){
        fields next=max;
        next.d[8]=a; next.d[9]=c; next.d[10]=d; roundtrip(&next);
    }
    for(uint32_t i=0;i<LFO;i++){
        uint32_t j=i, a=j%RATE; j/=RATE; uint32_t c=j%RATE; j/=RATE;
        uint32_t d=j%5; j/=5; uint32_t e=j%5,f=j/5;
        assert(a+RATE*(c+RATE*(d+5*(e+5*f)))==i);
        fields next=max; next.d[11]=i; roundtrip(&next);
    }
    deny(16,15,PAIR); deny(76,2,3); deny(82,21,LFO); deny(103,15,PAIR);
    deny(118,1,1); deny(119,1,1); deny(0,1,1); deny(8,1,1);
    fields unset={{0}};
    assert(!encode(b,sizeof b,&unset) && !memcmp(b,golden,sizeof b));
    memset(b,0,sizeof b); assert(!decode(&unset,b,sizeof b));
    puts("PASS 83,205 paired digit states, 36 selector states, 1,642,800 LFO states and malformed record refusals");
}
