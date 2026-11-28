#include <stdio.h>
#include "drum909.c"
int main(void) {
    uint32_t n=0, step=1, until=1u<<24;
    for(uint32_t x=0;; x+=step) {
        uint32_t ph=x>0x7fffffffu?0x7fffffffu:x;
        float t=d9_tri(ph);
        fm_bits_t a={.f=d9_diode20_phase(ph)}, b={.f=d9_tanh_inline(2.0f*t)*D9_INV_TANH_2_0};
        fm_bits_t c={.f=d9_diode16(t)}, d={.f=d9_tanh_inline(1.6f*t)*D9_INV_TANH_1_6};
        if(a.u!=b.u || c.u!=d.u) {
            fprintf(stderr,"FAIL %u %x %x %x %x\n",ph,a.u,b.u,c.u,d.u);
            return 1;
        }
        ++n;
        if(x==(1u<<31)) break;
        if(x==until) { step<<=1; until<<=1; }
    }
    printf("PASS all %u binary32-rounded triangle phase inputs, each for both fixed drives\n",n);
    return 0;
}
