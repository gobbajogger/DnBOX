#include "normal_coeff.h"
#include <stdio.h>
#include <assert.h>
int main(void) {
    {
        const sj6_normal_coeff x=sj6_normal_coefficients(44100.,699.264,0.4043673574924469);
        assert(x.b0==0x1.3f6c2ef090a00p-9);
        assert(x.b1==0x1.3f6c2ef090a00p-8);
        assert(x.b2==0x1.3f6c2ef090a00p-9);
        assert(x.a1==-0x1.f4a092ca2cf2cp+0);
        assert(x.a2==0x1.ee3ed6501c281p-1);
    }
    {
        const sj6_normal_coeff x=sj6_normal_coefficients(44100.,229.779,0.4043673574924469);
        assert(x.b0==0x1.17d8055433800p-12);
        assert(x.b1==0x1.17d8055433800p-11);
        assert(x.b2==0x1.17d8055433800p-12);
        assert(x.a1==-0x1.fcc55c4eac5eep+0);
        assert(x.a2==0x1.fa16a4a002d78p-1);
    }
    {
        const sj6_normal_coeff x=sj6_normal_coefficients(44100.,211.0,0.4043673574924469);
        assert(x.b0==0x1.d80ba5863c800p-13);
        assert(x.b1==0x1.d80ba5863c800p-12);
        assert(x.b2==0x1.d80ba5863c800p-13);
        assert(x.a1==-0x1.fd0dcf10ce967p+0);
        assert(x.a2==0x1.fa91a10afebbfp-1);
    }
    {
        const sj6_normal_coeff x=sj6_normal_coefficients(44100.,-1.0,0.4043673574924469);
        assert(x.b0==0x1.5fe4ad8680000p-20);
        assert(x.b1==0x1.5fe4ad8680000p-19);
        assert(x.b2==0x1.5fe4ad8680000p-20);
        assert(x.a1==-0x1.ffd7ea3ee83b1p+0);
        assert(x.a2==0x1.ffb0847027396p-1);
    }
    {
        const sj6_normal_coeff x=sj6_normal_coefficients(44100.,30000.0,0.4043673574924469);
        assert(x.b0==0x1.fd097be24ab16p-1);
        assert(x.b1==0x1.fd097be24ab16p+0);
        assert(x.b2==0x1.fd097be24ab16p-1);
        assert(x.a1==0x1.fce944d662023p+0);
        assert(x.a2==0x1.fa5365dc66c12p-1);
    }
    puts("PASS: 25 exact Float64 coefficients against five authentic old-AU probes");
}
