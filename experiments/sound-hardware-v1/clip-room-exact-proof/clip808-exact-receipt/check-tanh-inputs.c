#include <assert.h>
#include <stdio.h>
#include "fastmath.h"
int main(void)
{
    const fm_bits_t lo={.f=.0004f},hi={.f=0x1.760000p-5f};
    unsigned n=0;
    for(uint32_t i=lo.u;i<hi.u;++i)
        for(unsigned sign=0;sign<2;++sign) {
            const fm_bits_t v={.u=i|(sign<<31)};
            const float x=v.f,z=x*x;
            const fm_bits_t a={.f=fm_tanhf(x)};
            const fm_bits_t b={.f=x*(1.0f+z*(-0.333333333f+
                                  z*(0.133333333f+z*-0.0539682540f)))};
            assert(a.u==b.u);
            ++n;
        }
    printf("%u signed binary32 encodings in fast interval are exact\n",n);
    return 0;
}
