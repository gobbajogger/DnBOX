#include "actual-clip-key-host.inc"
int main(void) {
    static uint16_t want[240*82];
    for(unsigned theme=0;theme<4;theme++)for(unsigned rh=0;rh<16u;rh++){
        palette_set(theme); proj.set.ck_ch=2; proj.set.ck_gate=4;
        proj.set.ck_scale=rh%4u; proj.set.ck_root=rh%12u;
        proj.set.ck_x1=rh*3u; proj.set.ck_x2=(rh<<4)|rh;
        proj.set.ck_rate=8; proj.set.ck_oct=rh%3u;
        ck.on=CK_M_ARP|CK_M_SCALE|CK_M_CHORD; ck.pg=1u+rh%3u; ck.step=rh+1u;
        uint32_t sig=0;
        assert(smk_ck_panel(&sig,1u)); assert(cv_h==82u);
        memcpy(want,cv_px,sizeof want);
        packets=covered=mutations=0;
        assert(!smk_ck_panel(&sig,3u));
        assert(packets==28u && covered==82u && mutations==1u);
        assert(!memcmp(want,observed,sizeof want));
        assert(fwrite(want,sizeof want,1,stdout)==1);
    }
    return 0;
}
