/* SPDX-License-Identifier: GPL-3.0-only */
/* H1-H5 arp hooks (engine_arp.h) on the whole host app: Synth One and SuperJ6
 * arp on/off with held keys, UP/DOWN/UP+DN/SEQ order and timing, step API
 * round trip and stale revisions, H3 view, project storage incl. old projects,
 * SJ6 preset SEQ order vs the private reference bank (skips when absent) and
 * AMP2 119-122 storage. Also FW Core's extra: engine_param_si cutoff in Hz for
 * Chicago id 2 and Miami id 9 from each engine's own filter law. */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>
#include <dirent.h>
#include "app/original_runtime.h"
#include <math.h>
#include "chicago_packed.h"
#include "motion_tables_private.h"   /* original_miami_a44100 (static, read-only) */
#include "app/unit_control.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: CHECK(%s)\n", __FILE__, __LINE__, #x); exit(1); } } while (0)

static uint64_t arp_t_frame;
static struct { uint64_t at; int key, vel; } ev[4096];
static unsigned nev;
static void record(int key, int vel) { if (nev < 4096) { ev[nev].at = arp_t_frame; ev[nev].key = key; ev[nev].vel = vel; ++nev; } }
int __real_s1_part_note(s1_dry_part *, unsigned, unsigned);
int __wrap_s1_part_note(s1_dry_part *p, unsigned key, unsigned vel) { record((int)key, (int)vel); return __real_s1_part_note(p, key, vel); }
int __real_juno6_note_on(juno6_t *, uint8_t, uint8_t);
int __wrap_juno6_note_on(juno6_t *s, uint8_t key, uint8_t vel) { record(key, vel); return __real_juno6_note_on(s, key, vel); }
void __real_juno6_note_off(juno6_t *, uint8_t);
void __wrap_juno6_note_off(juno6_t *s, uint8_t key) { record(key, 0); __real_juno6_note_off(s, key); }
static juno6_t *last_juno;
static unsigned juno_opens;
void __real_juno6_init(juno6_t *, float);
void __wrap_juno6_init(juno6_t *s, float rate) { __real_juno6_init(s, rate); last_juno = s; ++juno_opens; }

static unsigned fine = 1;    /* 1-frame calls: event frames are exact */
static void render(unsigned frames)
{
    int32_t buf[512];
    while (frames) { unsigned n = fine ? 1 : frames > 256 ? 256 : frames; engine_render(buf, n); arp_t_frame += n; frames -= n; }
}
static void block(void) { render(256); }
static void select_bank(unsigned part, unsigned bank, unsigned preset)
{
    CHECK(engine_bass_select_preset(part, bank, preset, &proj.sound) == 0);
    for (unsigned i = 0; i < 1000 && engine_bass_preset_pending(part); ++i) block();
    CHECK(!engine_bass_preset_pending(part) && engine_bass_bank(part) == (int)bank);
}
static engine_arp_settings settings(unsigned v) { engine_arp_settings s; CHECK(engine_arp_settings_read(v, &s)); return s; }
static void set(unsigned v, unsigned field, int value)
{
    CHECK(engine_arp_set(v, settings(v).revision, field, value) == S1_EDIT_QUEUED);
    block();
}
static void step_set(unsigned v, unsigned step, unsigned field, int value)
{
    engine_arp_steps st; CHECK(engine_arp_steps_read(v, &st));
    CHECK(engine_arp_step_set(v, st.revision, step, field, value) == S1_EDIT_QUEUED);
}
static uint32_t period(void) { return engine_arp_step_frames(seq_tempo(&seq), ARP_SPEED_1_16); }
/* Note-ons in order (velocity > 0), from event index `from`. */
static unsigned ons(unsigned from, int *out, uint64_t *at, unsigned max)
{
    unsigned n = 0;
    for (unsigned i = from; i < nev && n < max; ++i) if (ev[i].vel) { out[n] = ev[i].key; if (at) at[n] = ev[i].at; ++n; }
    return n;
}
static void expect_order(unsigned v, const int *keys, unsigned nkeys, const int *want, unsigned nwant)
{
    nev = 0;
    for (unsigned i = 0; i < nkeys; ++i) engine_bass_on(v, keys[i], 0, 0);
    unsigned from = nev;
    render(period() * nwant + 64);
    int got[64]; uint64_t at[64];
    unsigned n = ons(from, got, at, 64);
    CHECK(n >= nwant);
    for (unsigned i = 0; i < nwant; ++i) {
        if (got[i] != want[i]) { fprintf(stderr, "step %u got %d want %d\n", i, got[i], want[i]); CHECK(0); }
        if (i) CHECK(at[i] >= at[i - 1] && (at[i] - at[i - 1]) % period() == 0);   /* rests: whole steps */
    }
    engine_bass_off(v);
    render(512);
}

static void s1_arp(void)
{
    const unsigned v = 0;
    select_bank(v, ORIGINAL_SOUND_SYNTHONE, 0);
    engine_arp_settings s = settings(v);
    CHECK(!s.on && s.mode == ARP_UP && s.range == 1 && s.speed == ARP_SPEED_1_16 && s.length == 16 &&
          s.interval == 12 && s.gate == 50 && !s.note_locked);
    engine_arp_steps st; CHECK(engine_arp_steps_read(v, &st) && st.on_mask == 0xFFFF && !st.oct_mask);
    /* OFF: held keys play directly. */
    unsigned from = nev;
    engine_bass_on(v, 48, 0, 0); engine_bass_on(v, 52, 0, 0); block();
    int got[64]; unsigned n = ons(from, got, NULL, 64);
    CHECK(n == 2 && got[0] == 48 && got[1] == 52);
    /* ON while held: direct notes end, the arp takes over from step 0. */
    from = nev;
    set(v, ARP_F_ON, 1);
    CHECK(ev[from].key == 48 && !ev[from].vel && ev[from + 1].key == 52 && !ev[from + 1].vel);
    render(period() * 4);
    n = ons(from, got, NULL, 64);
    CHECK(n >= 4 && got[0] == 48 && got[1] == 52 && got[2] == 48);
    /* Gate 50 %: each note ends half a step after it starts. */
    for (unsigned i = from; i + 1 < nev; ++i) if (ev[i].vel) {
        unsigned j = i + 1; while (j < nev && ev[j].key != ev[i].key) ++j;
        if (j < nev && (ev[j].vel || ev[j].at - ev[i].at != period() / 2)) fprintf(stderr, "on %d@%llu next %d vel %d @%llu period %u\n", ev[i].key, (unsigned long long)ev[i].at, ev[j].key, ev[j].vel, (unsigned long long)ev[j].at, period());
        if (j < nev) CHECK(!ev[j].vel && ev[j].at - ev[i].at == period() / 2);
        break;
    }
    /* H3: running view, last fired step and its age. */
    unsigned step; uint32_t age;
    CHECK(engine_arp_view(v, &step, &age) && step < 16 && age <= period() / 44u + 1);
    /* OFF while held: keys sound directly again. */
    from = nev;
    set(v, ARP_F_ON, 0);
    n = ons(from, got, NULL, 64);
    CHECK(n == 2 && got[0] == 48 && got[1] == 52);
    CHECK(!engine_arp_view(v, &step, &age));
    engine_bass_off(v); render(512);
    /* Modes, range and interval. */
    set(v, ARP_F_ON, 1);
    CHECK(!engine_arp_view(v, &step, &age));            /* on, but no keys held */
    set(v, ARP_F_RANGE, 2);
    { int k[] = {48, 52}, w[] = {48, 52, 60, 64, 48}; expect_order(v, k, 2, w, 5); }
    set(v, ARP_F_MODE, ARP_DOWN);
    { int k[] = {48, 52}, w[] = {64, 60, 52, 48, 64}; expect_order(v, k, 2, w, 5); }
    set(v, ARP_F_MODE, ARP_UPDN); set(v, ARP_F_INTERVAL, 7);
    { int k[] = {48, 52}, w[] = {48, 52, 55, 59, 55, 52, 48, 52}; expect_order(v, k, 2, w, 8); }
    /* SEQ: step offsets (S1 clamps to +-12), off steps rest, OCT adds 12, length wraps. */
    set(v, ARP_F_MODE, ARP_SEQ); set(v, ARP_F_LENGTH, 4);
    step_set(v, 0, ARP_S_OFFSET, 0); step_set(v, 1, ARP_S_OFFSET, 3); step_set(v, 2, ARP_S_ON, 0);
    step_set(v, 3, ARP_S_OFFSET, 20); step_set(v, 3, ARP_S_OCT, 1);
    block();
    CHECK(engine_arp_steps_read(v, &st) && st.offset[1] == 3 && st.offset[3] == 12 && !(st.on_mask & 4) && st.oct_mask == 8);
    { int k[] = {48}, w[] = {48, 51, 72, 48, 51}; expect_order(v, k, 1, w, 5); }
    /* SEQ plays every held key. */
    { int k[] = {48, 55}, w[] = {48, 55, 51, 58}; expect_order(v, k, 2, w, 4); }
    /* H3 follows the step that really sounded: rests do not move it. */
    engine_bass_on(v, 48, 0, 0);
    render(period() * 2 + 8);                      /* fired steps 0, 1; step 2 is off */
    CHECK(engine_arp_view(v, &step, &age) && step == 1 && age >= period() / 44u);
    render(period());                              /* step 3 fires */
    CHECK(engine_arp_view(v, &step, &age) && step == 3 && age < period() / 44u);
    engine_bass_off(v); render(512);
    /* Domain, stale revision, wrong voice. */
    s = settings(v);
    CHECK(engine_arp_set(v, s.revision, ARP_F_RANGE, 5) == S1_EDIT_REJECTED);
    CHECK(engine_arp_set(v, s.revision, ARP_NF, 0) == S1_EDIT_REJECTED);
    CHECK(engine_arp_step_set(v, s.revision, 16, ARP_S_ON, 1) == S1_EDIT_REJECTED);
    CHECK(engine_arp_set(v, s.revision - 1, ARP_F_ON, 0) == S1_EDIT_REJECTED);
    CHECK(engine_arp_set(2, s.revision, ARP_F_ON, 0) == S1_EDIT_REJECTED);
    /* Coalescing: two queued edits before one boundary, the newest wins. */
    CHECK(engine_arp_set(v, s.revision, ARP_F_LENGTH, 7) == S1_EDIT_QUEUED);
    CHECK(engine_arp_set(v, s.revision, ARP_F_LENGTH, 9) == S1_EDIT_QUEUED);
    CHECK(settings(v).length == 9 && settings(v).revision == s.revision);
    block();
    CHECK(settings(v).length == 9 && settings(v).revision == s.revision + 1);
    puts("synth-arp: S1 off/on with held keys, UP/DOWN/UPDN/SEQ order+timing, gate, H3 view, domains, revision PASS");
}

static void storage(void)
{
    const unsigned v = 0;
    engine_arp_settings before = settings(v);
    engine_arp_steps sb; CHECK(engine_arp_steps_read(v, &sb));
    engine_arp_cfg stored;
    CHECK(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0 + v], &stored));
    CHECK(stored.on == before.on && stored.mode == ARP_SEQ && stored.length == 9 && stored.offset[3] == 12 && stored.oct_mask == 8);
    /* Codec round trip over every field domain. */
    engine_arp_cfg c, d; uint8_t b[ARP_BLOCK_BYTES];
    for (unsigned i = 0; i < 2000; ++i) {
        engine_arp_cfg_default(&c);
        c.on = i & 1; c.mode = (uint8_t)(i % 4); c.range = (uint8_t)(1 + i % 4); c.speed = (uint8_t)(i % 10);
        c.length = (uint8_t)(1 + i % 16); c.interval = (uint8_t)(i % 13); c.gate = (uint8_t)(1 + i % 100);
        c.src_type = (uint8_t)(i % 7); c.amp2_mask = (uint8_t)(i % 16);
        for (unsigned s = 0; s < 16; ++s) c.offset[s] = (int8_t)((int)((i * 7 + s * 3) % 49) - 24);
        c.on_mask = (uint16_t)(i * 40503u); c.oct_mask = (uint16_t)(i * 2654435761u >> 9);
        for (unsigned a = 0; a < 4; ++a) c.amp2[a] = (uint16_t)(i * 977u + a * 13u);
        engine_arp_block_encode(&c, b);
        CHECK(engine_arp_block_decode(b, &d) && !memcmp(&c, &d, sizeof c));
    }
    /* Synth One source parameters (S1Parameter.h ids) through the decoder. */
    engine_arp_cfg_default(&c);
    CHECK(engine_arp_from_s1(&c, 62, 1) && engine_arp_from_s1(&c, 60, 2) && engine_arp_from_s1(&c, 63, 3) &&
          engine_arp_from_s1(&c, 61, 7) && engine_arp_from_s1(&c, 66, 5));
    CHECK(c.on && c.mode == ARP_DOWN && c.range == 4 && c.interval == 7 && c.length == 5);
    CHECK(engine_arp_from_s1(&c, 65, 1) && c.mode == ARP_SEQ && engine_arp_from_s1(&c, 60, 1) && c.mode == ARP_SEQ);
    CHECK(engine_arp_from_s1(&c, 65, 0) && c.mode == ARP_UPDN);
    CHECK(engine_arp_from_s1(&c, 67 + 4, -30) && c.offset[4] == -12 && engine_arp_from_s1(&c, 83 + 4, 1) && (c.oct_mask & 16));
    CHECK(engine_arp_from_s1(&c, 99 + 4, 0) && !(c.on_mask & 16) && engine_arp_from_s1(&c, 64, 120) && !engine_arp_from_s1(&c, 59, 0));
    /* Project save -> change -> load restores the arp and bumps the revision. */
    CHECK(project_save() == 0);
    sound_t saved = proj.sound;
    set(v, ARP_F_LENGTH, 3); set(v, ARP_F_ON, 0);
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    block();
    engine_arp_settings after = settings(v);
    engine_arp_steps sa; CHECK(engine_arp_steps_read(v, &sa));
    CHECK(after.on == before.on && after.mode == before.mode && after.length == before.length &&
          after.range == before.range && after.interval == before.interval && after.gate == before.gate);
    CHECK(!memcmp(sa.offset, sb.offset, sizeof sa.offset) && sa.on_mask == sb.on_mask && sa.oct_mask == sb.oct_mask);
    CHECK(after.revision != before.revision);
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    /* Older project: the cells are zero (never written) -> defaults, still valid. */
    memset(proj.sound.v[T_303][ARP_BLOCK_VOICE0], 0, ARP_BLOCK_BYTES * NBASS);
    CHECK(engine_bass_sound_validate(&proj.sound) == 0);
    CHECK(project_save() == 0);
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    block();
    after = settings(v);
    CHECK(!after.on && after.mode == ARP_UP && after.length == 16 && after.gate == 50);
    /* Unknown block version -> defaults too. */
    proj.sound.v[T_303][ARP_BLOCK_VOICE0][1] = 2;
    CHECK(!engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0], &d) && !d.on && d.length == 16);
    memset(proj.sound.v[T_303][ARP_BLOCK_VOICE0], 0, ARP_BLOCK_BYTES);
    /* A preset change keeps the arp. */
    set(v, ARP_F_ON, 1); set(v, ARP_F_MODE, ARP_DOWN);
    select_bank(v, ORIGINAL_SOUND_SYNTHONE, 0);
    CHECK(settings(v).on && settings(v).mode == ARP_DOWN);
    set(v, ARP_F_ON, 0);
    /* Undo path: reload re-queues the stored block. */
    proj.sound.v[T_303][ARP_BLOCK_VOICE0][2] |= 1u;
    CHECK(engine_arp_reload(v) == S1_EDIT_QUEUED); block();
    CHECK(settings(v).on);
    set(v, ARP_F_ON, 0);
    puts("synth-arp: storage codec x2000, project save/load, old project (zero block) and unknown version -> defaults, preset keeps arp, reload PASS");
}

/* ---- SuperJ6 */
static int sj6_id(const char *name)
{
    static const struct { const char *n; int id; } t[] = {
        {"arpEnable", 63}, {"arpType", 64}, {"arpOctave", 65}, {"arpInterval", 66}, {"attack2", 119},
        {"decay2", 120}, {"sustain2", 121}, {"release2", 122}, {"seqEnable", 142}, {"seqDivision", 143},
        {"seqGate", 144}, {"seqTotalSteps", 145}};
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; ++i) if (!strcmp(name, t[i].n)) return t[i].id;
    int n; char kind[16];
    if (sscanf(name, "seqStep%d%15s", &n, kind) == 2 && n >= 1 && n <= 16)
        return !strcmp(kind, "Offset") ? 146 + n : !strcmp(kind, "Toggle") ? 162 + n : -1;
    return -1;
}
typedef struct { float v[256]; uint8_t has[256]; } sj6_text;
static int sj6_read(const char *path, sj6_text *t)
{
    FILE *f = fopen(path, "r");
    char line[256];
    if (!f) return 0;
    memset(t, 0, sizeof *t);
    while (fgets(line, sizeof line, f)) {
        char name[128]; float x;
        if (sscanf(line, "neon%127s %f", name, &x) != 2) continue;
        int id = sj6_id(name);
        if (id >= 0 && !t->has[id]) { t->v[id] = x; t->has[id] = 1; }
    }
    fclose(f);
    return 1;
}
static void sj6_arp(void)
{
    const unsigned v = 1;
    select_bank(v, ORIGINAL_SOUND_JUNO, 0);
    engine_arp_settings s = settings(v);
    CHECK(!s.on && s.mode == ARP_UP);
    unsigned from = nev;
    engine_bass_on(v, 48, 0, 0); block();
    int got[64]; CHECK(ons(from, got, NULL, 64) == 1 && got[0] == 48);
    engine_bass_off(v); render(4096);
    set(v, ARP_F_ON, 1); set(v, ARP_F_RANGE, 3);
    { int k[] = {48, 52, 55}, w[] = {48, 52, 55, 60, 64, 67, 72, 76, 79, 48}; expect_order(v, k, 3, w, 10); }
    set(v, ARP_F_MODE, ARP_UPDN); set(v, ARP_F_RANGE, 1);
    { int k[] = {48, 52, 55}, w[] = {48, 52, 55, 52, 48, 52}; expect_order(v, k, 3, w, 6); }
    set(v, ARP_F_MODE, ARP_SEQ); set(v, ARP_F_LENGTH, 3);
    step_set(v, 0, ARP_S_OFFSET, -24); step_set(v, 1, ARP_S_OFFSET, 24); step_set(v, 2, ARP_S_OFFSET, 5);
    block();
    { int k[] = {60}, w[] = {36, 84, 65, 36}; expect_order(v, k, 1, w, 4); }   /* SJ6 keeps +-24 */
    puts("synth-arp: SJ6 off/on, UP range 3, UPDN, SEQ +-24 PASS");

    /* Reference: every private-bank preset's arp/seq fields through the decoder,
     * and the SEQ order of every seqEnable preset played on the SJ6 owner. */
    const char *dir = "/workspace/sj6-private/presets_txt";
    DIR *d = opendir(dir);
    fine = 0;
    if (!d) { puts("synth-arp: SJ6 reference SKIP (private bank absent)"); return; }
    unsigned presets = 0, seqs = 0, arps = 0, notes = 0, amp2 = 0;
    struct dirent *e;
    static sj6_text t;
    while ((e = readdir(d))) {
        char path[512];
        if (!strstr(e->d_name, ".txt")) continue;
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        CHECK(sj6_read(path, &t));
        engine_arp_cfg c; engine_arp_cfg_default(&c);
        for (unsigned id = 0; id < 256; ++id) if (t.has[id]) CHECK(engine_arp_from_sj6(&c, id, t.v[id]));
        ++presets;
        int seq_on = t.v[142] >= .5f, arp_on = t.v[63] >= .5f;
        CHECK(c.on == (seq_on || arp_on));
        CHECK((c.mode == ARP_SEQ) == seq_on);
        CHECK(c.range == 1 + (int)(t.v[65] * 3 + .5f));
        if (t.has[119]) { CHECK(c.amp2_mask == 15); ++amp2; }
        if (arp_on) ++arps;
        if (!seq_on) continue;
        ++seqs;
        /* Independent expectation from the text: enabled steps in order. */
        unsigned len = (unsigned)(t.v[145] * 15 + 1.5f);
        CHECK(c.length == len);
        int want[32]; unsigned nw = 0;
        for (unsigned cycle = 0; cycle < 2; ++cycle)
            for (unsigned i = 0; i < len && nw < 32; ++i) {
                int toggle = !t.has[163 + i] || t.v[163 + i] >= .5f;
                float off = t.has[147 + i] ? t.v[147 + i] : .5f;
                if (toggle) want[nw++] = 48 + (int)lrintf((off - .5f) * 48.f);
            }
        CHECK(engine_arp_preset(v, &c) == S1_EDIT_QUEUED); block();
        CHECK(settings(v).mode == ARP_SEQ && settings(v).length == len);
        c.speed = 9;                     /* the order, not the rate, is under test */
        c.gate = 50;
        CHECK(engine_arp_preset(v, &c) == S1_EDIT_QUEUED); block();
        nev = 0;
        engine_bass_on(v, 48, 0, 0);
        unsigned f0 = nev;
        uint32_t per = engine_arp_step_frames(seq_tempo(&seq), 9);
        render(per * 2 * len + 64);
        engine_bass_off(v); render(4096);
        int g[64]; unsigned ng = ons(f0, g, NULL, 64);
        CHECK(ng >= nw);
        for (unsigned i = 0; i < nw; ++i) if (g[i] != want[i]) {
            fprintf(stderr, "%s step %u got %d want %d\n", e->d_name, i, g[i], want[i]); CHECK(0);
        }
        notes += nw;
    }
    closedir(d);
    CHECK(presets == 640 && seqs == 122 && arps == 110);
    printf("synth-arp: SJ6 reference presets=%u arp=%u seq=%u seq-notes=%u (order == reference) amp2-from-preset=%u PASS\n",
           presets, arps, seqs, notes, amp2);
}

static void sj6_amp2(void)
{
    const unsigned v = 1;
    set(v, ARP_F_ON, 0);
    /* An AMP2 edit is applied and stored in the voice block. */
    unsigned tries = 0;
    while (!engine_superj6_edit(v, ENGINE_SJ6_AMP2_ATTACK, .25f) && tries++ < 4000) block();   /* release tail */
    CHECK(tries < 4000);
    CHECK(engine_superj6_edit(v, ENGINE_SJ6_AMP2_RELEASE, .75f));
    block();
    engine_arp_cfg c;
    CHECK(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0 + v], &c));
    CHECK(c.amp2_mask == 9 && c.amp2[0] == (uint16_t)(.25f * 65535.f + .5f) && c.amp2[3] == (uint16_t)(.75f * 65535.f + .5f));
    /* Project save/load re-stages AMP2 on the new owner. */
    CHECK(project_save() == 0);
    unsigned opens = juno_opens;
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    block(); block();
    CHECK(juno_opens > opens);
    float raw;
    CHECK(dnbx_read_raw(&last_juno->core, 119, &raw) == DNBX_OK && fabsf(raw - .25f) < 1e-4f);
    CHECK(dnbx_read_raw(&last_juno->core, 122, &raw) == DNBX_OK && fabsf(raw - .75f) < 1e-4f);
    CHECK(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0 + v], &c) && c.amp2_mask == 9);
    /* A preset change drops the stored AMP2 (the program owns it), keeps the arp. */
    set(v, ARP_F_ON, 1);
    select_bank(v, ORIGINAL_SOUND_JUNO, 1);
    CHECK(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0 + v], &c) && !c.amp2_mask && c.on);
    puts("synth-arp: SJ6 AMP2 119-122 edit -> block, project reload re-stages, preset change clears PASS");
}

/* One-pole z += A(z' - z): |H(w)| = (1-A) / |1 - A e^-jw| (libm, double). */
static double pole_gain(double a, double hz)
{
    double w = 2.0 * M_PI * hz / 44100.0;
    return (1.0 - a) / sqrt(1.0 - 2.0 * a * cos(w) + a * a);
}
static void cutoff_hz(void)
{
    static const struct { unsigned kind, id; const char *name; } e[2] = {
        {ORIGINAL_SOUND_CHICAGO, ORIGINAL_CHICAGO_CUTOFF, "Chicago"}, {ORIGINAL_SOUND_MIAMI, ORIGINAL_MIAMI_CUTOFF, "Miami"}};
    for (unsigned k = 0; k < 2; ++k) {
        select_bank(0, e[k].kind, 0);
        float hz[128], u;
        for (unsigned pot = 0; pot < 128; ++pot) {
            CHECK(engine_param_si(T_303, 0, (int)e[k].id, (int)pot, &hz[pot]) == X0X_U_HZ);
            CHECK(hz[pot] >= 0.0f && hz[pot] <= 22050.0f && (!pot || hz[pot] >= hz[pot - 1]));
            /* The real law, independently: program mapping -> native 24 ->
             * the engine's own A coefficient; the reported Hz is that pole's
             * corner (|H| = -3 dB within 2 % below 4 kHz) and -ln(A) fs/2pi in libm. */
            CHECK(original_sound_control_value(e[k].kind, e[k].id, pot, &u));
            float c, a, ee;
            if (e[k].kind == ORIGINAL_SOUND_CHICAGO) {
                original_chicago_program pr; memset(&pr, 0, sizeof pr);
                CHECK(original_chicago_program_set(&pr, 2, u) && original_chicago_program_native(&pr, 24, &c));
                CHECK(original_chicago_packed_lookup(0, (uint32_t)(c * 16384.0f), &a, &ee));
            } else {
                original_miami_program pr; memset(&pr, 0, sizeof pr);
                CHECK(original_miami_program_set(&pr, 9, u) && original_miami_program_native(&pr, 24, &c));
                float q = c * 128.0f; unsigned j = (unsigned)q;
                a = j >= 128 ? original_miami_a44100[128] :
                    original_miami_a44100[j] + (q - (float)j) * (original_miami_a44100[j + 1] - original_miami_a44100[j]);
            }
            double want = a > 0 ? -log((double)a) * 44100.0 / (2.0 * M_PI) : 22050.0;
            if (want > 22050.0) want = 22050.0;
            CHECK(fabs(hz[pot] - want) <= 0.002 * want + 0.01);
            if (want > 20.0 && want < 4000.0) CHECK(fabs(pole_gain(a, hz[pot]) - M_SQRT1_2) < 0.02 * M_SQRT1_2);
        }
        CHECK(hz[127] > 4.0f * hz[32]);
        /* Other controls keep their old (unitless) display. */
        float x;
        CHECK(engine_param_si(T_303, 0, (int)e[k].id + 1, 64, &x) == X0X_U_NONE);
        CHECK(engine_param_si(T_303, 0, (int)e[k].id, 128, &x) == X0X_U_NONE);
        printf("synth-arp: %s cutoff id %u pot 0/32/64/96/127 = %.1f/%.1f/%.1f/%.1f/%.1f Hz PASS\n",
               e[k].name, e[k].id, hz[0], hz[32], hz[64], hz[96], hz[127]);
    }
    select_bank(0, ORIGINAL_SOUND_SYNTHONE, 0);
    float x;
    CHECK(engine_param_si(T_303, 0, ORIGINAL_CHICAGO_CUTOFF, 64, &x) == X0X_U_HZ);   /* S1 id 2 = Crush rate, unchanged */
    CHECK(x == s1_part_crush_hz(unit_control(64)));
}

int main(void)
{
    boot();
    s1_arp();
    storage();
    sj6_arp();
    cutoff_hz();
#if X0X_SUPERJ6_FACTORY
    sj6_amp2();
#endif
    puts("synth-arp-hooks: PASS");
    return 0;
}
