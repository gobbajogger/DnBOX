#define X0X_IMA_TABLES_DEFINE
#include "ima_adpcm.h"
#include <stdio.h>
#define X0X_POOL
static int32_t d9_ima_transition[89][16] X0X_POOL;
static inline int32_t d9_ima_decode(const uint8_t *d, uint32_t i, int32_t *pred, int32_t *idx)
{
    uint32_t byte = d[i >> 1], code = (i & 1u) ? (byte >> 4) : (byte & 15u);
    const int32_t z = d9_ima_transition[*idx][code];
    int32_t p = *pred + (z >> 7);
    *pred = p < -32768 ? -32768 : p > 32767 ? 32767 : p;
    *idx = z & 127;
    return *pred;
}

int main(void)
{
    for (int ix=0; ix<89; ++ix)
        for (int code=0; code<16; ++code) {
            const int m=code&7, step=IMA_STEP_T[ix];
            const int delta=((step>>3) + ((m&1)?(step>>2):0) +
                             ((m&2)?(step>>1):0) + ((m&4)?step:0));
            int next=ix+IMA_IDX_T[m];
            if (next<0) next=0;
            if (next>88) next=88;
            d9_ima_transition[ix][code]=((code&8)?-delta:delta)*128+next;
        }
    unsigned long total=0;
    for (int k=0;k<89;++k) for (int c=0;c<16;++c)
        for (int s=-32768;s<=32767;++s) {
            uint8_t byte=(uint8_t)(c*17);
            int a=s,b=s,i=k,j=k;
            if (ima_decode(&byte,total&1,&a,&i)!=d9_ima_decode(&byte,total&1,&b,&j)
                || a!=b || i!=j) return 1;
            ++total;
        }
    printf("PASS %lu exact IMA predictor/index transitions\n",total);
    return 0;
}
