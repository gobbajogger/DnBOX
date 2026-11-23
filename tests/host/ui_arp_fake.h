/* SPDX-License-Identifier: GPL-3.0-only */
/* Host fake of Synths' engine_arp.h (ARP-HOOKS-API.md, f68c624e): same types,
 * enums and signatures; state is local and applied at once (one boundary). */
#pragma once
#include <stdint.h>
enum { ARP_STEPS = 16 };
enum { ARP_UP, ARP_DOWN, ARP_UPDN, ARP_SEQ, ARP_NMODES };
enum { ARP_NSPEED = 10, ARP_SPEED_1_16 = 8 };
typedef struct { uint32_t revision; uint8_t on, mode, range, speed, length, interval, note_locked, gate; } engine_arp_settings;
typedef struct { uint32_t revision; int8_t offset[ARP_STEPS]; uint16_t on_mask, oct_mask; } engine_arp_steps;
enum { ARP_F_ON, ARP_F_MODE, ARP_F_RANGE, ARP_F_SPEED, ARP_F_LENGTH, ARP_F_INTERVAL, ARP_F_GATE, ARP_NF };
enum { ARP_S_OFFSET, ARP_S_ON, ARP_S_OCT };
#if !X0X_SYNTHONE_PATCH
enum { S1_EDIT_REJECTED, S1_EDIT_QUEUED, S1_EDIT_BUSY };
#endif
static struct {
    int ok[2];                  /* voice's engine is SJ6/S1 */
    int offmax[2];              /* 12 S1, 24 SJ6 */
    engine_arp_settings s[2];
    engine_arp_steps st[2];
    unsigned view_step[2]; uint32_t view_age[2]; int view_ok[2];
    unsigned sets, step_sets, rejected;
} arp_fake;
static void arp_fake_reset(void)
{
    for (int v = 0; v < 2; v++) {
        arp_fake.ok[v] = 1; arp_fake.offmax[v] = 12;
        arp_fake.s[v] = (engine_arp_settings){1, 0, ARP_UP, 1, ARP_SPEED_1_16, 16, 12, 0, 50};
        arp_fake.st[v] = (engine_arp_steps){1, {0}, 0xFFFFu, 0};
        arp_fake.view_ok[v] = 0;
    }
    arp_fake.sets = arp_fake.step_sets = arp_fake.rejected = 0;
}
static int arp_fake_clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
int engine_arp_settings_read(unsigned voice, engine_arp_settings *out)
{
    if (voice > 1 || !arp_fake.ok[voice]) return 0;
    *out = arp_fake.s[voice]; return 1;
}
int engine_arp_steps_read(unsigned voice, engine_arp_steps *out)
{
    if (voice > 1 || !arp_fake.ok[voice]) return 0;
    *out = arp_fake.st[voice]; out->revision = arp_fake.s[voice].revision; return 1;
}
int engine_arp_set(unsigned voice, uint32_t revision, unsigned field, int value)
{
    engine_arp_settings *s;
    if (voice > 1 || !arp_fake.ok[voice] || field >= ARP_NF) { arp_fake.rejected++; return S1_EDIT_REJECTED; }
    s = &arp_fake.s[voice];
    if (revision != s->revision) { arp_fake.rejected++; return S1_EDIT_REJECTED; }
    switch (field) {
    case ARP_F_ON: s->on = value != 0; break;
    case ARP_F_MODE: s->mode = (uint8_t)arp_fake_clamp(value, 0, ARP_NMODES - 1); break;
    case ARP_F_RANGE: s->range = (uint8_t)arp_fake_clamp(value, 1, 4); break;
    case ARP_F_SPEED: s->speed = (uint8_t)arp_fake_clamp(value, 0, ARP_NSPEED - 1); break;
    case ARP_F_LENGTH: s->length = (uint8_t)arp_fake_clamp(value, 1, 16); break;
    case ARP_F_INTERVAL: s->interval = (uint8_t)arp_fake_clamp(value, 0, 12); break;
    default: s->gate = (uint8_t)arp_fake_clamp(value, 1, 100); break;
    }
    s->revision++; arp_fake.sets++;
    return S1_EDIT_QUEUED;
}
int engine_arp_step_set(unsigned voice, uint32_t revision, unsigned step, unsigned field, int value)
{
    engine_arp_steps *st;
    if (voice > 1 || !arp_fake.ok[voice] || step >= ARP_STEPS || field > ARP_S_OCT ||
        revision != arp_fake.s[voice].revision) { arp_fake.rejected++; return S1_EDIT_REJECTED; }
    st = &arp_fake.st[voice];
    if (field == ARP_S_OFFSET)
        st->offset[step] = (int8_t)arp_fake_clamp(value, -arp_fake.offmax[voice], arp_fake.offmax[voice]);
    else {
        uint16_t *m = field == ARP_S_ON ? &st->on_mask : &st->oct_mask;
        *m = (uint16_t)(value ? *m | 1u << step : *m & ~(1u << step));
    }
    arp_fake.s[voice].revision++; arp_fake.step_sets++;
    return S1_EDIT_QUEUED;
}
int engine_arp_view(unsigned voice, unsigned *step, uint32_t *fired_age_ms)
{
    if (voice > 1 || !arp_fake.view_ok[voice]) return 0;
    *step = arp_fake.view_step[voice]; *fired_age_ms = arp_fake.view_age[voice]; return 1;
}
