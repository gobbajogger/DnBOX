#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern float original_ladder(float *,float,float,float);
extern float test_ladder(float *,float,float,float);
static uint32_t rng=0x717bb554u;
static uint32_t next(void) {
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng;
}
static float unit(void) {
    return (float)(next()>>8)*(1.0f/16777216.0f);
}
int main(void) {
    float a[5], b[5];
    for (unsigned j=0;j<1048576u;++j) {
        if ((j&127u)==0) {
            for (unsigned i=0;i<5;++i) a[i]=j&128u ? -0.0f : unit()*2.0f-1.0f;
            memcpy(b,a,sizeof a);
        }
        float in=unit()*2.f-1.f, coeff=unit(), fb=unit()*3.f;
        if(j%127u==0) in=-0.0f;
        float ao=original_ladder(a,in,coeff,fb);
        float bo=test_ladder(b,in,coeff,fb);
        if (memcmp(a,b,sizeof a)||memcmp(&ao,&bo,sizeof ao)) {
            fprintf(stderr,"Ladder output/state mismatch at %u\n",j); return 1;
        }
    }
    puts("1,048,576 ladder steps bit-identical for output and all five states");
}
