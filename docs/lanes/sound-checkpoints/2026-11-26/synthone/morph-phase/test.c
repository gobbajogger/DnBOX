#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "phase-round.h"
static void check(float x){
    assert(morph_round_phase24(x)==((uint32_t)lrintf(x)&0xffffffu));
}
int main(void){
    uint32_t n=0,s=1;
    for(unsigned e=119;e<=158;++e){
        for(unsigned mant=0;mant<=0x7fffffu;mant+=13u){
            union{uint32_t u;float f;} a={.u=(e<<23)|mant};check(a.f);check(-a.f);n+=2;
        }
    }
    for(unsigned i=0;i<4194304;++i){
        s=s*1664525u+1013904223u;
        check((float)(s & 0x3fffffu)*.25f);n++;
    }
    printf("%u matched binary32 cases\n",n);
}
