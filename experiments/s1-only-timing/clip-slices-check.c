#include <stdlib.h>
#include "actual-clip-key-host.inc"
#include <assert.h>
static void clip_fixture_mutate(void) {
    palette_set(0);
    proj.set.ck_gate ^= 3u;
    ck.on=0u; ck.pg=0u; ck.step=0u;
}
int main(void) {
    static uint16_t wanted[240*82];
    unsigned scenes=0;
    for(unsigned theme=0;theme<4;theme++) for(unsigned rh=0;rh<16;rh++) {
        palette_set((uint8_t)theme); proj.set.ck_ch=2; proj.set.ck_gate=4;
        proj.set.ck_scale=(uint8_t)(rh%4u); proj.set.ck_root=(uint8_t)(rh%12u);
        proj.set.ck_x1=(uint8_t)(rh*3u); proj.set.ck_x2=(uint8_t)((rh<<4)|rh);
        proj.set.ck_rate=8; proj.set.ck_oct=(uint8_t)(rh%3u);
        ck.on=CK_M_ARP|CK_M_SCALE|CK_M_CHORD; ck.pg=(uint8_t)(1u+rh%3u);
        ck.step=(uint8_t)(rh+1u);
        uint32_t sig=0;
        assert(smk_ck_panel(&sig,1u));
        assert(cv_h==82u);
        memcpy(wanted,cv_px,sizeof wanted);
        strips=0; adversarial=1;
        assert(!smk_ck_panel(&sig,3u));
        assert(strips==2u);
        assert(!memcmp(wanted,emitted,sizeof wanted));
        scenes++;
    }
    printf("PASS %u exact CLIP Keys footers, input and palette mutated after first strip\n",scenes);
}
