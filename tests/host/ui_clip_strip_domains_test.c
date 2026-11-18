/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include "actual-clip-key-host.inc"
static ck_t base_ck;
static settings_t base_settings;
static unsigned states;
static void reset(unsigned page) { ck=base_ck; proj.set=base_settings; ck.pg=page; }
static void sample(void) {
    ck_t saved=ck; settings_t settings=proj.set; uint32_t sig=0;
    unsigned usb=fake_tx_n, ble=ble_tx_n;
    assert(smk_ck_panel(&sig,1));
    assert(!memcmp(&saved,&ck,sizeof ck)); assert(!memcmp(&settings,&proj.set,sizeof settings));
    assert(usb==fake_tx_n && ble==ble_tx_n);
    unsigned ink=0;for(unsigned i=0;i<240*82;i++)ink+=cv_px[i]!=swap16(C_BLACK);
    assert(ink>0);assert(!smk_ck_panel(&sig,0));states++;
}
int main(void) {
    palette_set(1);proj.set.ck_ch=2;proj.set.ck_gate=4;proj.set.ck_rate=8;
    base_ck=ck;base_settings=proj.set;
    for(unsigned page=1;page<=3;page++)for(unsigned slot=0;slot<(page==3?9u:4u);slot++) {
        unsigned field=CK_PAGE_FIELDS[page-1][slot];
        for(unsigned raw=0;raw<=CK_FMAX[field];raw++) {
            reset(page);ck_set(field,raw);
            assert(field==CK_E_LATCH ? !!ck_get(field)==!!raw : ck_get(field)==raw);sample();
        }
    }
    assert(states==153);
    for(unsigned root=0;root<=CK_FMAX[CK_E_ROOT];root++)for(unsigned scale=0;scale<=CK_FMAX[CK_E_SCALE];scale++)for(unsigned snap=0;snap<=CK_FMAX[CK_E_SNAP];snap++) {
        reset(1);ck_set(CK_E_ROOT,root);ck_set(CK_E_SCALE,scale);ck_set(CK_E_SNAP,snap);ck_set(CK_E_ON,1);sample();
    }
    assert(states==153+576);
    for(unsigned order=0;order<=CK_FMAX[CK_E_ORD];order++)for(unsigned rate=0;rate<=CK_FMAX[CK_E_RATE];rate++)for(unsigned rhythm=0;rhythm<=CK_FMAX[CK_E_RHY];rhythm++) {
        reset(3);ck_set(CK_E_ORD,order);ck_set(CK_E_RATE,rate);ck_set(CK_E_RHY,rhythm);
        ck_set(CK_E_OCT,CK_FMAX[CK_E_OCT]);ck_set(CK_E_GATE,CK_FMAX[CK_E_GATE]);ck_set(CK_E_SWING,CK_FMAX[CK_E_SWING]);ck_set(CK_E_HUMAN,CK_FMAX[CK_E_HUMAN]);ck_set(CK_E_ON,1);sample();
    }
    assert(states==153+576+1248);
    printf("PASS actual strip: %u states; 153 single-field + 576 scale combinations + 1248 arp combinations at maximum auxiliary settings; no full-product or visual acceptance\n",states);
    return 0;
}
