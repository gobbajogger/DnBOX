/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

enum { FRAMES = 4096 };
static int32_t dry[FRAMES * 2], linked[FRAMES * 2], replay[FRAMES * 2];

static void visit(const char *title)
{
    for (int i = 0; i < pg.n; ++i) {
        if (!strcmp(page_title(&pg, cur_page()), title)) return;
        now_ms += 160;
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(0);
}

static void edit(int knob, int delta)
{
    now_ms += 160;
    turn(knob, delta);
}

#include "dsp/s1c/s1c.h"
static float actual_amp[4], actual_filter[4];
static unsigned observed;
void __real_s1c_render(s1c *,float *,float *,unsigned);
void __wrap_s1c_render(s1c *s,float *l,float *r,unsigned n)
{
    for(unsigned v=0;v<S1C_VOICES;v++) if(s->v[v].gate) {
        for(unsigned k=0;k<4;k++) {
            actual_amp[k]=s->t[s1_patch_at(S1_AMP_FIRST+k)->factory_id];
            actual_filter[k]=s->t[s1_patch_at(S1_FILTER_FIRST+k)->factory_id];
        }
        ++observed;break;
    }
    __real_s1c_render(s,l,r,n);
}
static void capture(unsigned owner, int32_t *pcm)
{
    engine_apply_sound(&proj.sound);
    render(4);
    observed=0;engine_bass_on(owner, 57, 0, 0);
    for (unsigned i = 0; i < FRAMES; i += 256) engine_render(pcm + i * 2, 256);
    CHECK(observed>0);
    engine_bass_off(owner);
    render(8);
}

static unsigned delta_pcm(const int32_t *a, const int32_t *b)
{
    unsigned changed = 0;
    for (unsigned i = 0; i < FRAMES * 2; ++i) changed += a[i] != b[i];
    return changed;
}

static void journey(unsigned owner)
{
    setup(V_PART, PART_303A + owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(!engine_bass_select_preset(1-owner, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(1-owner); ++n) render(1);
    CHECK(engine_bass_bank(1-owner)==ORIGINAL_SOUND_SYNTHONE);
    build_pages();
    unsigned nlink=0;
    for(int i=0;i<pg.n;i++) {
        CHECK(strcmp(page_title(&pg,i),"AMP LINK"));
        for(unsigned k=0;k<4;k++) if(pg.r[i][k].kind==R_S1LINK) {
            CHECK(k==3 && !strcmp(page_title(&pg,i),"FILTER"));
            nlink++;
        }
    }
    CHECK(nlink==1);
    visit("FILTER");
    pref_t link = pg.r[cur_page()][3];
    CHECK(pref_desc(link) && !strcmp(pref_desc(link)->name, "LINK"));
    CHECK(pref_get(link) == 0);
    uint8_t original[ORIGINAL_SOUND_BYTES];
    memcpy(original, proj.sound.v[T_303][owner], sizeof original);
    /* A LINK-only edit must survive recognition of historical slot-2 cells. */
    edit(3, 1);
    CHECK(!engine_bass_sound_validate(&proj.sound));
    CHECK(!project_save());
    CHECK(!project_load());
    engine_apply_sound(&proj.sound);
    CHECK(pref_get(link) == 1);
    edit(3, -1);
    CHECK(!memcmp(original, proj.sound.v[T_303][owner], sizeof original));

    edit(2, 70);                 /* audible filter envelope depth */
    visit("FILTER ADSR");
    edit(0, 40);
    capture(owner, dry);
    CHECK(memcmp(actual_amp,actual_filter,sizeof actual_amp));
    s1_patch_snapshot before, on, after;
    CHECK(engine_s1_patch_snapshot(owner, &before));
    CHECK(before.physical[S1_FILTER_FIRST] != before.physical[S1_AMP_FIRST]);
    const sound_t peer = proj.sound;
    s1_patch_snapshot peer_before, peer_after;
    int peer_is_s1 = engine_s1_patch_snapshot(1-owner,&peer_before);

    visit("FILTER");
    edit(3, 1);
    CHECK(engine_s1_patch_snapshot(owner, &on) && on.filter_link);
    CHECK(!memcmp(before.physical, on.physical, sizeof before.physical));
    capture(owner, linked);
    CHECK(!memcmp(actual_amp,actual_filter,sizeof actual_amp));
    unsigned changed = delta_pcm(dry, linked);
    CHECK(changed > FRAMES);
    for (unsigned row=1-owner;row<8;row+=2)
        CHECK(!memcmp(peer.v[T_303][row],proj.sound.v[T_303][row],NPARAMS_MAX));
    if (peer_is_s1) {
        CHECK(engine_s1_patch_snapshot(1-owner,&peer_after) && peer_before.filter_link==peer_after.filter_link);
        CHECK(!memcmp(peer_before.physical,peer_after.physical,sizeof peer_before.physical));
    }
    CHECK(!project_save());
    CHECK(!project_load());
    capture(owner, replay);
    CHECK(!delta_pcm(linked, replay));

    edit(3, -1);
    CHECK(engine_s1_patch_snapshot(owner, &after) && !after.filter_link);
    CHECK(!memcmp(before.physical, after.physical, sizeof before.physical));
    CHECK(!project_save() && !project_load());
    CHECK(pref_get(link)==0);
    capture(owner, replay);
    CHECK(!delta_pcm(dry, replay));
    CHECK(memcmp(actual_amp,actual_filter,sizeof actual_amp));
    printf("owner=%u filter LINK changed %u/%u PCM samples, reload and unlink bit-exact\n",
           owner, changed, FRAMES * 2);
}

int main(void)
{
    journey(0);
    journey(1);
    printf("PASS Synth One envelope LINK %u checks\n", checks);
    return 0;
}
