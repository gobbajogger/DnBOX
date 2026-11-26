#include "bridge48.h"
#include "generated_test_rows.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

#define N 4096
static float input[2][N], middle[2][2*N];
static void pass(void *state, float l, float r, float out[2])
{
    ++*(unsigned *)state;
    out[0]=l; out[1]=r;
}
int main(void)
{
    float max = 0;
    const sj6_bridge48_coef coef = COEFS;
    sj6_bridge48 c = {0};
    unsigned calls=0;
    for (int i=0;i<N;i++) {
        input[0][i] = .125f*(float)sin(.027*i) + (i==256 ? .0625f : 0.f);
        input[1][i] = .098f*(float)cos(.392*i) - .009f*(float)cos(1.31*i);
    }
    for (int i=0;i<2*N-1;i++) for (int ch=0;ch<2;ch++) {
        int q=i/2;
        if (!(i&1)) middle[ch][i]=q>=32?input[ch][q-32]:0;
        else for (int j=-31;j<=32;++j) {
            int x=q-32+j;
            if (x>=0) middle[ch][i] += exact_up[j+31]*input[ch][x];
        }
    }
    for (int i=0;i<N;i++) {
        float out[2];
        sj6_bridge48_tick(&c,&coef,pass,&calls,input[0][i],input[1][i],out);
        assert(calls == (unsigned)(2*i+1));
        for (int ch=0;ch<2;ch++) {
            float wanted=0;
            for (int j=-31;j<=32;j++) {
                int x=2*i-32+j;
                if(x>=0) wanted += exact_down[j+31]*middle[ch][x];
            }
            float delta=fabsf(out[ch]-wanted);
            if(delta>max) max=delta;
        }
    }
    assert(max<2.e-7f);
    printf("PASS: stereo factored vs 64-tap reference, %u callbacks, max abs %.9g\n",calls,max);
}
