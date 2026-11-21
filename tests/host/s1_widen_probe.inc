/* SPDX-License-Identifier: GPL-3.0-only */
/* Test-only observer, linked against the archived original Soundpipe port.c. */
#include "soundpipe.h"
static sp_data port_data;
static sp_port original_port[2];
static struct {
    uint32_t samples[2205];
    unsigned cursor, resets, frames, audible, dry_taps;
} width_probe[2];
static uint32_t wbits(float x) { uint32_t b; memcpy(&b, &x, 4); return b; }

void s1_widen_cold(int v)
{
    port_data.sr = 44100;
    assert(sp_port_init(&port_data, &original_port[v], .1f) == SP_OK);
    memset(width_probe[v].samples, 0, sizeof width_probe[v].samples);
    width_probe[v].cursor = 0;
    ++width_probe[v].resets;
}

void s1_widen_observe(int v, float input, float tap, float target, float applied, float output)
{
    float smooth;
    assert(sp_port_compute(&port_data, &original_port[v], &target, &smooth) == SP_OK);
    assert(wbits(applied) == wbits(smooth));
    unsigned i = width_probe[v].cursor;
    assert(wbits(tap) == (width_probe[v].samples[i] & UINT32_C(0xffffff00)));
    width_probe[v].samples[i] = wbits(input);
    width_probe[v].cursor = (i + 1) % 2205;
    ++width_probe[v].frames;
    float expected = applied * tap + (1.f - applied) * input;
    assert(wbits(output) == wbits(applied == 0.f ? input : expected));
    if (fabsf(tap) > .00001f && wbits(input) != wbits(tap)) {
        if (applied > 0.f && wbits(output) != wbits(input)) ++width_probe[v].audible;
        else if (applied == 0.f) ++width_probe[v].dry_taps;
    }
}
static void wnote(int v, int pitch, int velocity)
{
    uint32_t g=0,c=0; int arp=0;
    engine_live_key_state(v,&g,&arp,&c);
    assert(engine_live_key(v,g,2,pitch,velocity,0,0));
}
static void repeat_blocks(unsigned n) { while(n--) block(); }

static void width_scenario(void)
{
    select_patch(0, ORIGINAL_SOUND_SYNTHONE); select_patch(1, ORIGINAL_SOUND_SYNTHONE);
    s1_patch_value a = read_patch(0,121), b = read_patch(1,121);
    assert(a.physical == 0 && b.physical == 0);
    /* Create and actually reload a prior 13-slot project cell. */
    assert(engine_s1_patch_set(0,a.revision,16,3) == S1_EDIT_QUEUED);
    block();
    unsigned width_byte = S1_PATCH_FIRST_BYTE + S1_PATCH_LEGACY_COUNT;
    assert(!(original_sound_edit_mask(proj.sound.v[T_303][0]) & (1u << width_byte)));
    assert(project_save() == 0 && project_load() == 0);
    engine_apply_sound(&proj.sound); block();
    a = read_patch(0,121); b = read_patch(1,121);
    assert(!a.physical && !b.physical && read_patch(0,16).position == 3);
    wnote(0,47,99); wnote(1,62,83); repeat_blocks(17);
    assert(width_probe[0].dry_taps > 512 && width_probe[1].dry_taps > 512);
    unsigned aw = width_probe[0].audible, bw = width_probe[1].audible;
    assert(engine_s1_patch_set(0,a.revision,121,127) == S1_EDIT_QUEUED);
    repeat_blocks(2);
    assert(width_probe[0].audible - aw > 350 && width_probe[1].audible == bw);
    assert(engine_s1_patch_set(1,b.revision,121,64) == S1_EDIT_QUEUED);
    repeat_blocks(10);
    assert(width_probe[1].audible - bw > 2100 && original_port[1].yt1 < read_patch(1,121).physical);
    struct { s1_patch_snapshot snapshot; uint32_t canary; } legacy;
    legacy.canary = UINT32_C(0x0dd57eaf);
    assert(engine_s1_patch_snapshot(1,&legacy.snapshot));
    assert(legacy.canary == UINT32_C(0x0dd57eaf));
    float amount;
    assert(engine_s1_patch_snapshot_widen(1,&legacy.snapshot,&amount));
    assert(legacy.canary == UINT32_C(0x0dd57eaf) && amount == read_patch(1,121).physical);
    assert(project_save() == 0);
    sound_t saved = proj.sound;
    unsigned resets_b = width_probe[1].resets;
    wnote(0,47,0); select_patch(0, ORIGINAL_SOUND_CHICAGO);
    assert(!engine_s1_patch_read(0,121,&a));
    unsigned a_frames = width_probe[0].frames, b_frames = width_probe[1].frames;
    repeat_blocks(16);
    assert(width_probe[0].frames == a_frames && width_probe[1].frames == b_frames + 4096);
    select_patch(0, ORIGINAL_SOUND_SYNTHONE);
    assert(read_patch(0,121).physical == 0.f && read_patch(1,121).position == 64);
    assert(width_probe[1].resets == resets_b);
    assert(project_load() == 0);
    engine_apply_sound(&proj.sound); block();
    assert(!memcmp(&saved,&proj.sound,sizeof saved));
    assert(read_patch(0,121).position == 127 && read_patch(1,121).position == 64);
    assert(!engine_s1_patch_set(1,b.revision,121,3));
    printf("width words checked A=%u B=%u, audible A=%u B=%u, dry warm A=%u B=%u, resets A=%u B=%u\n",
           width_probe[0].frames,width_probe[1].frames,width_probe[0].audible,width_probe[1].audible,
           width_probe[0].dry_taps,width_probe[1].dry_taps,width_probe[0].resets,width_probe[1].resets);
}
