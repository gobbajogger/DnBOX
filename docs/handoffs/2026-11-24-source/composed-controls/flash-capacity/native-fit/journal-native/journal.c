#include "journal.h"
#include "crc32_nibble.h"
#define VMJ_MAGIC UINT32_C(0x314a4e44)
#define X0X_MAGIC UINT32_C(0x30583058)

_Static_assert(2*(48+32)+4*16==VMJ_BODY,"serialization size");
_Static_assert(VMJ_BODY<=VMJ_PAGE && VMJ_TEMP>=VMJ_PAGE+48,"temporary staging disjoint");
_Static_assert(VMJ_TEMP+2*VMJ_PAGE<=4096,"temporary staging within legacy sector");
_Static_assert(sizeof(vmj_snapshot)==200,"native fixed state size");
_Static_assert(VMJ_PANEL_B-VMJ_PANEL_A==4096,"panel pair exactly 8 KiB");

static __attribute__((noinline)) uint32_t rd32(const uint8_t *p)
{ return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24; }
static __attribute__((noinline)) void wr32(uint8_t *p,uint32_t v)
{ for(unsigned i=0;i<4;i++){p[i]=(uint8_t)v;v>>=8;} }
static void bytes(uint8_t *d,const uint8_t *s,unsigned n)
{ while(n--) *d++=*s++; }
static const uint8_t starts[4]={0,48,96,128};
static unsigned cap(unsigned i){return i<2 ? 48u : 32u;}
static unsigned equal(const uint8_t *a,const uint8_t *b,unsigned n)
{while(n--)if(*a++!=*b++)return 0;return 1;}

static int load_v1(vmj_read r,void*cx,uint32_t off,uint8_t*h,uint8_t *body,vmj_snapshot *result)
{
    if(r(cx,off,h,VMJ_HEADER))return -2;
    if(rd32(h)!=VMJ_MAGIC)return 0;
    unsigned n=rd32(h+8), gen=rd32(h+4);
    if(!gen||n<64||n>VMJ_BODY)return 0;
    if(r(cx,off+VMJ_PAGE,body,n))return -2;
    if(~x0x_crc32_upd(x0x_crc32_upd(UINT32_MAX,h,28),body,n)!=rd32(h+28))return 0;
    uint8_t *p=body;
    const uint8_t *end=body+n;
    /* Validate the entire tuple before updating the caller's snapshot. */
    unsigned lens[4];uint32_t seq[4];
    for(unsigned i=0;i<4;i++){
        if((unsigned)(end-p)<16||rd32(p)!=i+1)return 0;
        unsigned m=rd32(p+8);
        uint32_t s=rd32(p+4);
        if(m>cap(i)||(unsigned)(end-p)-16<m||(!m&&s)||x0x_crc32(p+16,m)!=rd32(p+12))return 0;
        lens[i]=m;seq[i]=s;p+=16+m;
    }
    if(p!=end)return 0;
    if(result->generation){
        if(!equal(result->binding,h+12,16))return -1;
        if(result->generation>=gen)return 1;
    }
    bytes(result->binding,h+12,16);
    result->generation=gen;
    p=body;
    for(unsigned i=0;i<4;i++){
        unsigned m=lens[i];p+=16;
        result->len[i]=(uint8_t)m;result->seq[i]=seq[i];
        bytes(result->data+starts[i],p,m);p+=m;
    }
    return 1;
}

static int legacy_one(vmj_read r,void *cx,unsigned off,unsigned kind,unsigned limit,
                      uint8_t*h, uint8_t*data,uint32_t *seq)
{
    if(r(cx,off,h,VMJ_HEADER))return -2;
    unsigned erased=1;
    for(unsigned i=0;i<VMJ_HEADER;i++) erased&=(h[i]==255);
    if(erased)return 0;
    unsigned n=rd32(h+12);
    if(rd32(h)!=X0X_MAGIC || h[4]!=kind||h[5]||!n||n>limit||x0x_crc32(h,28)!=rd32(h+28))return -1;
    if(r(cx,off+VMJ_PAGE,data,n))return -2;
    if(x0x_crc32(data,n)!=rd32(h+16))return -1;
    *seq=rd32(h+8);
    return (int)n;
}

int vmj_select(vmj_read r,void*cx,vmj_snapshot*out)
{
    uint8_t h[VMJ_HEADER], body[VMJ_BODY];
    out->generation=0;
    for(unsigned phase=0;phase<=VMJ_TEMP;phase+=VMJ_TEMP){
        int v=load_v1(r,cx,VMJ_SET_A+phase,h,body,out);
        if(v<0)return v;
        v=load_v1(r,cx,VMJ_SET_B+phase,h,body,out);
        if(v<0)return v;
        if(out->generation)return 1;
    }
    static const uint8_t old_owner[16]="SMK37DATAX0X";
    if(r(cx,VMJ_OWNER,h,16))return -2;
    if(!equal(h,old_owner,16))return -1;
    for(unsigned i=0;i<4;i+=2){
        unsigned off=i?VMJ_PANEL_A:VMJ_SET_A;
        uint32_t q[2]={0,0};
        int a=legacy_one(r,cx,off,i?4:0,cap(i),h,body,q);
        int b=legacy_one(r,cx,off+4096,i?4:0,cap(i),h,body+48,q+1);
        if(a<0||b<0)return a<0?a:b;
        unsigned swap=!a || (b&&q[1]>q[0]);
        if(a&&b&&q[0]==q[1]){
            if(a!=b||!equal(body,body+48,(unsigned)a))return -1;
            b=0;q[1]=0;
        }
        for(unsigned at=0;at<2;at++){
            unsigned k=i+at, pick=at^swap, n=pick?(unsigned)b:(unsigned)a;
            out->len[k]=(uint8_t)n;out->seq[k]=q[pick];
            bytes(out->data+starts[k],body+48*pick,n);
        }
    }
    return 0;
}

int vmj_encode(const vmj_snapshot*s,uint8_t h[VMJ_HEADER],uint8_t body[VMJ_BODY])
{
    if(!s->generation)return -1;
    unsigned off=0;
    for(unsigned i=0;i<4;i++){
        unsigned n=s->len[i];
        const uint8_t *data=s->data+starts[i];
        if(n>cap(i)||(!n&&s->seq[i]))return -1;
        wr32(body+off,i+1);wr32(body+off+4,s->seq[i]);
        wr32(body+off+8,n);wr32(body+off+12,x0x_crc32(data,n));
        bytes(body+off+16,data,n);off+=16+n;
    }
    wr32(h,VMJ_MAGIC);wr32(h+4,s->generation);wr32(h+8,off);bytes(h+12,s->binding,16);
    wr32(h+28,~x0x_crc32_upd(x0x_crc32_upd(UINT32_MAX,h,28),body,off));
    return (int)off;
}
