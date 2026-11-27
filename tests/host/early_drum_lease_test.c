/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include "smk_d9_early_route.h"

int main(void)
{
    const uint8_t black[11]={1,3,5,8,10,13,15,17,20,22,25};
    d9_lease l={0};
    d9_admit out;
    unsigned accepted=0;
    d9_lease_open(&l,1,0,0);
    for (unsigned k=0;k<37;++k) for(unsigned vel=0;vel<129;++vel) {
        d9_postscan s={.down_after=UINT64_C(1)<<k};
        unsigned v=0;
        while(v<11 && black[v]+5u!=k) ++v;
        const int hit=d9_scan_admit(&l,&s,k,vel,black,&out);
        assert(hit==(v<11 && vel>0 && vel<=127));
        if(hit) { assert(out.physical==k && out.voice==v && out.velocity==vel); ++accepted; }
    }
    for(unsigned f=0;f<256;++f) {
        d9_postscan s={.down_before=f&1u,.down_after=(UINT64_C(1)<<10)|((f>>1)&1u),
                       .button_epoch=(f>>2)&1u,.button_now=f&8u,.pad_notes=f&16u,
                       .debounce=f&32u,.encoder_pending=f&64u,.other_pending=f&128u};
        assert(d9_scan_admit(&l,&s,10,127,black,&out)==(f==0));
    }
    d9_postscan s={.down_after=UINT64_C(1)<<10};
    /* Mask has returned to zero, but two edges occurred while foreground stalled. */
    s.button_epoch=d9_epoch_next(d9_epoch_next(0));
    assert(!d9_scan_admit(&l,&s,10,127,black,&out));
    d9_lease_open(&l,1,0,s.button_epoch);
    assert(!d9_scan_admit(&l,&s,10,127,black,&out));
    d9_lease_open(&l,1,s.button_epoch,s.button_epoch);
    assert(d9_scan_admit(&l,&s,10,127,black,&out));
    d9_lease_close(&l);
    assert(!d9_scan_admit(&l,&s,10,127,black,&out));
    l.last_id=UINT32_MAX-1;
    d9_lease_open(&l,1,0,0);
    assert(!l.live_id && l.last_id==UINT32_MAX);
    assert(d9_epoch_next(UINT32_MAX)==UINT32_MAX);
    printf("%u physical strike combinations, 256 rejection masks and epoch/Panic-route preconditions pass\n",accepted);
    return 0;
}
