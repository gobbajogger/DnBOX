#include "drum909.h"
extern drum909_t d909;
static float bus[4][256];
static volatile uint32_t *const result=(void*)0x1c07f00;
static uint32_t checksum; /* Rolling hash, seeded to zero by the fixture. */
__attribute__((noinline)) static void measured(void) {
    for(unsigned block=0;block<4;block++) {
        for(unsigned j=0;j<4;++j)for(unsigned k=0;k<256;++k) bus[j][k]=0.0f;
        drum909_render_st_cleared(&d909,bus[0],bus[1],bus[2],bus[3],256,2047);
        for(unsigned j=0;j<4;++j)for(unsigned k=0;k<256;++k) {
            union { float f; uint32_t u; } bits={.f=bus[j][k]};
            checksum=(checksum^bits.u)*16777619u;
        }
    }
}
void bench(void) {
    drum909_init(&d909);
    for(unsigned i=0;i<DR_NUM;++i)
        if((MASK>>i)&1) drum909_trigger(&d909,i,.875f);
    measured();
    result[0]=MASK; result[1]=checksum; result[2]=d909.smp[0].mute; result[3]=d909.tom[0].mute;
}
