/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

enum { FRAMES = 4096 };
static int32_t pcm[2][FRAMES * 2];

static void capture(unsigned owner, int32_t *out, int knob, int delta)
{
    engine_apply_sound(&proj.sound);
    render(4);
    engine_bass_on(owner, 57, 0, 0);
    render(4);
    if (knob >= 0) {
        now_ms += 160;
        turn(knob, delta);
    } else {
        render(4);
    }
    for (unsigned i = 0; i < FRAMES; i += 256)
        engine_render(out + i * 2, 256);
    engine_bass_off(owner);
    render(8);
}

static void select_page(const char *title)
{
    for (int i = 0; i < pg.n; ++i) {
        if (!strcmp(page_title(&pg, cur_page()), title)) return;
        now_ms += 160;
        enc_acc[EN_SELECT] = 1;
        input();
    }
}

static void change(unsigned owner, unsigned knob, int delta, const char *label)
{
    pref_t p = pg.r[cur_page()][knob];
    CHECK(pref_desc(p) && !strcmp(pref_desc(p)->name, label));
    int before = pref_get(p);
    capture(owner, pcm[0], -1, 0);
    capture(owner, pcm[1], (int)knob, delta);
    CHECK(pref_get(p) != before);
    unsigned changed = 0;
    for (unsigned i = 0; i < FRAMES * 2; ++i)
        changed += pcm[0][i] != pcm[1][i];
    CHECK(changed > FRAMES);
    printf("owner=%u %s position=%d->%d changed PCM samples=%u/%u\n",
           owner, label, before, pref_get(p), changed, FRAMES * 2);
}

static void journey(unsigned owner)
{
    setup(V_PART, PART_303A + owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, 0, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_JUNO);
    build_pages();
    sound_t before = proj.sound;
    select_page("OSCILLATOR");
    change(owner, 0, 1, "WAVE 1");
    change(owner, 1, 1, "WAVE 2");
    change(owner, 2, 1, "RANGE");
    if (!owner) { ui_frame(); shot("oscillator-smk"); }
    select_page("PULSE WIDTH");
    now_ms += 160; turn(1, 20);
    now_ms += 160; turn(3, 25);
    change(owner, 0, 1, "PWM 1 ON");
    change(owner, 2, 1, "PWM 2 ON");
    change(owner, 1, -8, "PWM 1");
    change(owner, 3, -10, "PWM 2");
    CHECK(!memcmp(before.v[T_303][1-owner], proj.sound.v[T_303][1-owner], ORIGINAL_SOUND_BYTES));
    CHECK(!memcmp(before.v[T_303][ARP_BLOCK_VOICE0 + owner],
                  proj.sound.v[T_303][ARP_BLOCK_VOICE0 + owner], sizeof proj.sound.v[T_303][0]));
    if (!owner) { ui_frame(); shot("pwm-smk"); }
    CHECK(!project_save());
    sound_t saved = proj.sound;
    capture(owner, pcm[0], -1, 0);
    CHECK(!project_load());
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    capture(owner, pcm[1], -1, 0);
    CHECK(!memcmp(pcm[0], pcm[1], sizeof pcm[0]));
    CHECK(engine_superj6_knob_turn(owner, SJ6_PWM1_ON, -1, 1));
    CHECK(project_save() == -8);
    /* Init-only controls build: the absent factory slot must not mutate Sound
       or cancel the already queued edit; an explicit Init resets the editor. */
    CHECK(engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, 1, &proj.sound) == -1);
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    CHECK(project_save() == -8);
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, 0, &proj.sound));
    CHECK(!engine_superj6_knob_turn(owner, SJ6_PWM2_ON, -1, 1));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner));
    CHECK(engine_superj6_knob_position(owner, SJ6_PWM1_ON) == 0);
    CHECK(!original_sound_edit_mask(proj.sound.v[T_303][owner]));
    printf("owner=%u exact project reload PCM and preset reset PASS\n", owner);
}

int main(void)
{
    journey(0);
    journey(1);
    engine_case(ORIGINAL_SOUND_SYNTHONE, "s1");
    engine_case(ORIGINAL_SOUND_JUNO, "sj6");
    CHECK(!failures);
    printf("PASS SuperJ6 knob playback/project %u and existing arp %u checks\n", checks, expectations);
    return 0;
}
