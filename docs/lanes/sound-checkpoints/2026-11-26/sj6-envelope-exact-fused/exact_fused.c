/* Exact source recurrence of admitted finite normal binary64 operands.
 * Positive mul and state; additive term may have either sign.
 * Reject alignment/cancellation outside explicitly supported range. */
#include <stdint.h>
typedef union {double d; uint64_t u;} Bits;
typedef uint32_t Limb;
enum {N=6};
static unsigned bitlen(const Limb *w)
{
    for(unsigned k=N;k--;)
        if(w[k])return 32*k+32-__builtin_clz(w[k]);
    return 0;
}
static void shift(Limb *dst,const Limb *w,unsigned n,unsigned d)
{
    unsigned lo=d/32,b=d%32;
    for(unsigned k=0;k<n;++k){
        if(k+lo<N)dst[k+lo]|=w[k]<<b;
        if(b && k+lo+1<N)dst[k+lo+1]|=w[k]>>(32-b);
    }
}
static unsigned bit(const Limb *w,unsigned u){return (w[u/32]>>(u%32))&1;}
int sj6_fused(double m,double y,double a,double *out)
{
    const uint64_t frac=(UINT64_C(1)<<52)-1, one=UINT64_C(1)<<52;
    Bits bm={m},by={y},ba={a};
    unsigned em=(bm.u>>52)&2047,ey=(by.u>>52)&2047,ea=(ba.u>>52)&2047;
    if((bm.u|by.u)>>63 || !em || em==2047 || ey==2047 || ea==2047)return 0;
    if(!ey){if(by.u)return 0;*out=a;return 1;}
    if(!ea && (ba.u&frac))return 0;
    uint64_t im=(bm.u&frac)|one,iy=(by.u&frac)|one,ia=(ba.u&frac)|one;
    Limb ml[2]={(Limb)im,(Limb)(im>>32)},yl[2]={(Limb)iy,(Limb)(iy>>32)};
    Limb p[4]={0},q[2]={(Limb)ia,(Limb)(ia>>32)},x[N]={0},z[N]={0};
    for(unsigned i=0;i<2;++i){
        uint64_t carry=0;
        for(unsigned j=0;j<2;++j){
            uint64_t t=(uint64_t)ml[i]*yl[j]+carry+p[i+j];
            p[i+j]=(Limb)t;carry=t>>32;
        }
        p[i+2]=(Limb)carry;
    }
    const int ep=(int)(em+ey)-2150,eadd=(int)ea-1075;
    const int e0=ea&&eadd<ep?eadd:ep;
    unsigned dp=ep-e0,da=ea?eadd-e0:0;
    if(dp>85 || da>138)return 0;
    shift(x,p,4,dp);
    if(ea)shift(z,q,2,da);
    unsigned s=ba.u>>63;
    if(!ea)s=0;
    if(s){
        unsigned borrow=0;
        for(unsigned k=0;k<N;++k){
            unsigned b=x[k]<z[k] || (borrow && x[k]==z[k]);
            x[k]-=z[k]+borrow;borrow=b;
        }
        if(borrow){unsigned carry=1;for(unsigned k=0;k<N;++k){
            x[k]=~x[k]+carry;carry=carry&&!x[k];
        }}else s=0;
    }else{
        unsigned carry=0;
        for(unsigned k=0;k<N;++k){
            uint64_t u=(uint64_t)x[k]+z[k]+carry;x[k]=(Limb)u;carry=u>>32;
        }
        if(carry)return 0;
    }
    unsigned n=bitlen(x);
    if(!n){*out=0.;return 1;}
    int en=e0+(int)n-1;
    uint64_t r=0;
    for(unsigned i=0;i<53 && i<n;++i)
        r|=(uint64_t)bit(x,n-1-i)<<(52-i);
    if(n>53 && bit(x,n-54)){
        unsigned sticky=r&1;
        for(unsigned i=0;i<n-54&&!sticky;++i)sticky|=bit(x,i);
        r+=sticky;
        if(r==2*one){r=one;++en;}
    }
    if(en<=-1023||en>=1024)return 0;
    Bits br={.u=((uint64_t)s<<63)|((uint64_t)(en+1023)<<52)|(r&frac)};
    *out=br.d;return 1;
}
