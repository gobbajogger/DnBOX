/* SPDX-License-Identifier: GPL-3.0-only */
/* Live production PART pages; fixed evidentiary lower bound, not AU parity. */
#define ARP_FIXTURE_MAIN retained_arp_inventory
#include "ui_arp_engine_test.c"
#include "app/s1c_part.h"

static int mapped_juno(unsigned slot)
{
    switch (slot) {
    case SJ6_WAVE1: return 82;
    case SJ6_WAVE2: return 83;
    case SJ6_RANGE: return 86;
    case SJ6_PWM1_ON: return 210;
    case SJ6_PWM1: return 78;
    case SJ6_PWM2_ON: return 211;
    case SJ6_PWM2: return 79;
    case SJ6_FILTER_ATTACK: return 132;
    case SJ6_FILTER_DECAY: return 133;
    case SJ6_FILTER_SUSTAIN: return 134;
    case SJ6_FILTER_RELEASE: return 135;
    case SJ6_FILTER_LINK: return 116;
    case SJ6_MASTER_LEVEL: return 0;
    case SJ6_LFO1_RATE: return 88;
    case SJ6_LFO2_RATE: return 96;
    case SJ6_LFO3_RATE: return 100;
    case SJ6_LFO2_PWM_DEPTH: return 97;
    case SJ6_LFO1_MODE: return 195;
    case SJ6_LFO2_MODE: return 196;
    case SJ6_LFO3_MODE: return 197;
    case SJ6_CHORUS_I: return 40;
    case SJ6_CHORUS_II: return 41;
    case SJ6_VIBRATO_DEPTH: return 105;
    case SJ6_VIBRATO_RATE: return 106;
    case SJ6_LFO2_SHAPE: return 98;
    case SJ6_LAYER1_SEMI: return 72;
    case SJ6_LAYER2_SEMI: return 73;
    case SJ6_LAYER1_FINE: return 74;
    case SJ6_LAYER2_FINE: return 75;
    case SJ6_OSC1_LEVEL: return 1;
    case SJ6_OSC2_LEVEL: return 3;
    case SJ6_A_REFERENCE: return 206;
    default: return -1;
    }
}
int main(void)
{
    const unsigned kinds[]={ORIGINAL_SOUND_SYNTHONE,ORIGINAL_SOUND_JUNO};
    const char *const names[]={"Synth One","SuperJ6"};
    for (unsigned i=0;i<2;++i) for (unsigned v=0;v<NBASS;++v) {
        setup(V_PART,PART_303A+v);
        CHECK(!engine_bass_select_preset(v,kinds[i],0,&proj.sound));
        for (unsigned k=0;k<1024 && engine_bass_preset_pending(v);++k) render(1);
        CHECK(engine_bass_bank(v)==(int)kinds[i]);
        build_pages();
        unsigned count=0; uint8_t seen[256]={0};
        printf("%s %u: %d live pages\n",names[i],v,pg.n);
        for (int p=0;p<pg.n;++p) for (int k=0;k<4;++k) {
            pref_t r=pg.r[p][k];
            if (pref_read_only(r)) continue;
            int id=-1;
            if (i==0 && r.kind==R_S1PATCH) {
                const s1_patch_descriptor *d=s1_patch_at(r.c);
                if (d) id=d->factory_id;
            }
            if (i==0 && r.kind==R_ENG && r.a==T_303 && r.c==ORIGINAL_SYNTHONE_CRUSH) id=34;
            if (i==1 && r.kind==R_SJ6KNOB) id=mapped_juno(r.c);
            if (i==1 && r.kind==R_ENG && r.a==T_303 && r.c==9) id=28;
            if (id<0) continue;
            CHECK(id<256);
            if (!seen[id]) { seen[id]=1; ++count; }
            printf("  id=%d page=%s K%d\n",id,page_title(&pg, p),k+1);
        }
        CHECK(count==(i?33u:55u));
        printf("Direct mapped distinct IDs: %u\n",count);
    }
}
