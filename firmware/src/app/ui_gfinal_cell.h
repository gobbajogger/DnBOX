/* SPDX-License-Identifier: GPL-3.0-only */
/* G-FINAL in place: descriptor -> shared glyph type, live adapters, and the
 * 26x19 cell compositor. Included by ui.c after the tape/tempo glyph state.
 * Rows the map does not admit (HOLD/DYNAMIC/READ_ONLY) return 0 and keep 0.0.26. */
#ifndef UI_GFINAL_CELL_H
#define UI_GFINAL_CELL_H
#include "ui_gfinal.h"
#include "ui_gfinal_map.h"

static UI_DRAW int drum_live(pref_t r,uint32_t *age,float *env);

static UI_DRAW int gf_glyph_of(pref_t r, const x0x_param_t *d)
{
    if (r.kind == R_GEN) return gf_gen_glyph[r.c & 7] == 255 ? -1 : gf_gen_glyph[r.c & 7];
    if (r.kind != R_ENG) return r.kind < GF_KIND_N && gf_kind_glyph[r.kind] != 255 ? gf_kind_glyph[r.kind] : -1;
    if (!d || !d->name) return -1;
    char k[6] = {0}; int j = 0;
    for (const char *n = d->name; *n && j < 6; n++) {
        char c = *n; if (c >= 'A' && c <= 'Z') c += 32;
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) k[j++] = c;
    }
    int drum = r.a == T_909 || r.a == T_808;
    /* the five names whose type depends on the engine (parameter-map.json) */
    if (!memcmp(k, "attack", 6)) return drum ? GF_TRANSIENT : GF_ATTACK;
    if (!memcmp(k, "tune\0\0", 6)) return r.a == T_909 && r.b == 0 ? GF_PITCH_DECAY : GF_TUNE;
    if (!memcmp(k, "pitch\0", 6)) return r.a == T_909 ? GF_PITCH_BASE : GF_TUNE;
    if (!memcmp(k, "tone\0\0", 6)) return r.a == T_808 ? (r.b == 0 ? GF_CUTOFF : r.b == 1 ? GF_BLEND : -1)
                                         : r.a == T_909 && r.b == 1 ? GF_DECAY : -1;
    if (!memcmp(k, "filter", 6)) return r.a == T_MST && r.c == 8 ? GF_FILTER_MODE : GF_CUTOFF;
    int lo = 0, hi = GF_NAME_N - 1;
    while (lo <= hi) {
        int m = (lo + hi) >> 1, c = memcmp(k, gf_name_key[m], 6);
        if (!c) return gf_name_glyph[m];
        if (c < 0) hi = m - 1; else lo = m + 1;
    }
    return -1;
}

/* Live adapters: only measured engine/transport state, never wall-clock. */
static UI_DRAW void gf_live_of(pref_t r, gf_live *L)
{
    memset(L, 0, sizeof *L);
    if (r.kind == R_TAPE) { L->tape_active = tape_glyph_running; L->tape_phase = (gf)tape_glyph_phase / 16; return; }
    if (r.kind == R_TEMPO) { L->playing = tempo_glyph_view != 0; L->clock_phase = tempo_glyph_view ? (gf)(tempo_glyph_view - 1) / 32 : 0; return; }
    if (r.kind != R_ENG) return;
    if (r.a == T_303) {
        float a = 0, m = 0, eg = 0, dd = 0, ph = 0, bi = 0, cu = 0, tg = 0; unsigned s = 6, h = 0; int on = 0;
        if (engine_bass_voice_view(r.b, &a, &m, &s, &h) && s != 6) { L->active = 1; L->amp = a; L->env = a; L->progress = 1 - a; L->mod = m; }
        if (engine_bass_filter_view(r.b, &eg, &dd)) L->filter_env = eg;
        if (engine_bass_motion_view(r.b, &ph, &bi, &cu, &tg, &on) && on && ph >= -1 && ph <= 1) { L->phase = ph; L->mod = bi; }
    } else if (r.a == T_909 || r.a == T_808 || r.a == T_MIX) {
        uint32_t age = 0xffffffffu; float e = 0;
        if (drum_live(r, &age, &e)) { L->active = 1; L->amp = e; L->env = e; L->progress = 1 - e; }
    }
}

/* Composite one shared glyph into the 26x19 cell at (gx,gy). The 988 B coverage
 * mask lives on this stack frame (no static RAM). */
static UI_DRAW __attribute__((noinline)) void gf_draw(int id, int v, int st, const gf_live *L, int gx, int gy, uint16_t ink)
{
    uint16_t mask[GF_W * GF_H];
    const uint16_t col[4] = { C_LINE, C_GRAY, ink, C_WHITE };
    if (v < 0) v = 0; if (v > 127) v = 127;
    for (int role = 0; role < 4; ++role) {
        gf_render(id, v, st, L, role, mask);
        for (int i = 0; i < GF_W * GF_H; ++i)
            if (mask[i]) px_blend(gx + i % GF_W, gy + i / GF_W, col[role], (uint32_t)__builtin_popcount(mask[i]));
    }
}
/* A row drawn as a given type: its value, enumerated state and live engine view. */
static UI_DRAW void gf_cell_as(pref_t r, int id, int val, int gx, int gy, uint16_t ink)
{
    const x0x_param_t *d = pref_desc(r);
    int max = d && d->max ? d->max : 127, st = -1;
    if (gf_states[id] && val >= 0 && val <= max && max + 1 == gf_states[id]) st = val;
    gf_live L; gf_live_of(r, &L);
    gf_draw(id, (val * 127 + max / 2) / max, st, &L, gx, gy, ink);
}
/* The row's own type from its descriptor; 0 when the map has no type for it. */
static UI_DRAW int gf_cell(pref_t r, int val, int gx, int gy, uint16_t ink)
{
    int id = gf_glyph_of(r, pref_desc(r));
    if (id < 0) return 0;
    gf_cell_as(r, id, val, gx, gy, ink);
    return 1;
}
/* An ordinary knob strip: the shared cell centred on cx, rows 29..47. */
static UI_DRAW int control_glyph(pref_t r, int val, int cx, uint16_t ink) { return gf_cell(r, val, cx - 13, 29, ink); }
/* The Synth One filter readout: live cutoff/resonance drive the shared curve. */
static UI_DRAW void gf_filt_cell(int gx, int gy, int mode, int cutoff, int resonance, uint16_t ink)
{
    gf_live L; memset(&L, 0, sizeof L);
    L.has_cutoff = L.has_resonance = 1; L.cutoff = (gf)cutoff / 127; L.resonance = (gf)resonance / 127;
    gf_draw(GF_CUTOFF, cutoff, -1, &L, gx, gy, ink); (void)mode;
}
#endif
