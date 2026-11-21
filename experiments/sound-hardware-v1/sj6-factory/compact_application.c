/* Fractional-coverage factory test for the compact SuperJ6 application tree.
 * The generated data comes from the owner's original bank export at replay. */
#define SMC_UI_MAIN retained_ui_tests
#include "smc_ui_test.c"
#include "app/original_runtime.h"
#include "bank_reader.c"

static juno6_t *owner[2];
static unsigned inits;

void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *part, float rate)
{
    __real_juno6_init(part, rate);
    if (inits < 2u) owner[inits++] = part;
}

typedef struct {
    float master, adsr[4], wave[2];
    engine_superj6_pwm pwm[2];
    unsigned present;
} app_fields;

static void open_parts(void)
{
    int32_t stereo[512];
    setup(V_PART, PART_303A);
    for (int v = 0; v < 2; ++v) {
        CHECK(engine_bass_select_preset(v, ORIGINAL_SOUND_JUNO, v, &proj.sound) == 0);
        for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(v); ++n)
            engine_render(stereo, 256);
        CHECK(engine_bass_bank(v) == ORIGINAL_SOUND_JUNO && !engine_bass_preset_pending(v));
    }
    CHECK(inits == 2 && owner[0] != owner[1]);
}

static int select_fields(unsigned bank, unsigned position, app_fields *patch)
{
    sj6_record_reader reader;
    if (!sj6_select_position(bank, position, &reader)) return 0;
    /* Build 9 resolves each missing PWM amount and enable independently to
     * literal binary32 +0 in its main preset pass. Keep authored presence
     * separate so a preset switch resets live state without adding fields
     * to the original serialized record. */
    memset(patch, 0, sizeof *patch);
    unsigned id;
    uint32_t bits;
    while (sj6_next_field(&reader, &id, &bits)) {
        float x;
        memcpy(&x, &bits, sizeof x);
        if (id == ENGINE_SJ6_MASTER_VOLUME) {
            patch->master = x; patch->present |= 1u << 10;
        } else if (id >= 119 && id <= 122) {
            patch->adsr[id-119] = x; patch->present |= 1u << (id-119);
        } else if (id == 82 || id == 83) {
            patch->wave[id-82] = x; patch->present |= 1u << (4+id-82);
        } else if (id == 78 || id == 79) {
            patch->pwm[id-78].amount = x; patch->present |= 1u << (6+id-78);
        } else if (id == 210 || id == 211) {
            patch->pwm[id-210].enable = x; patch->present |= 1u << (8+id-210);
        }
    }
    return (patch->present & 1087u) == 1087u; /* master, AMP2 and waves */
}

static void check_raw(unsigned v, unsigned id, float expected)
{
    float applied;
    CHECK(dnbx_read_raw(&owner[v]->core, id, &applied) == DNBX_OK);
    CHECK(memcmp(&applied, &expected, sizeof applied) == 0);
}

static void apply_fields(unsigned v, const app_fields *loaded, unsigned *authored, unsigned *defaulted)
{
    uint32_t old_presence[7];
    float old_raw[218];
    memcpy(old_presence, owner[v^1]->core.raw_present, sizeof old_presence);
    memcpy(old_raw, owner[v^1]->core.raw, sizeof old_raw);
    CHECK(engine_superj6_edit(v, ENGINE_SJ6_MASTER_VOLUME, loaded->master));
    for (unsigned k = 0; k < 4; ++k)
        CHECK(engine_superj6_edit(v, (enum engine_superj6_field)(119+k), loaded->adsr[k]));
    for (unsigned l = 0; l < 2; ++l)
        CHECK(engine_superj6_pre_note_wave(v, l, loaded->wave[l], &loaded->pwm[l]));
    int32_t stereo[2];
    engine_render(stereo, 1);
    check_raw(v, ENGINE_SJ6_MASTER_VOLUME, loaded->master);
    for (unsigned k = 0; k < 4; ++k)
        check_raw(v, 119 + k, loaded->adsr[k]);
    *authored += 7; /* master, four AMP2 controls, and two wave selectors */
    for (unsigned l = 0; l < 2; ++l) {
        check_raw(v, 82 + l, loaded->wave[l]);
        check_raw(v, 78 + l, loaded->pwm[l].amount);
        check_raw(v, 210 + l, loaded->pwm[l].enable);
        const unsigned pwm_authored = ((loaded->present >> (6+l)) & 1u)
                                    + ((loaded->present >> (8+l)) & 1u);
        *authored += pwm_authored;
        *defaulted += 2u - pwm_authored;
    }
    CHECK(!memcmp(old_raw, owner[v^1]->core.raw, sizeof old_raw));
    CHECK(!memcmp(old_presence, owner[v^1]->core.raw_present, sizeof old_presence));
}

#ifndef SJ6_APPLICATION_MAIN
#define SJ6_APPLICATION_MAIN main
#endif
int SJ6_APPLICATION_MAIN(void)
{
    unsigned applied = 0, refused = 0, authored = 0, defaulted = 0;
    open_parts();
    for (unsigned b = 0; b < BANK_COUNT; ++b) {
        for (unsigned p = 0; p < (unsigned)(bank_start[b+1]-bank_start[b]); ++p) {
            app_fields loaded;
            if (!select_fields(b, p, &loaded)) { ++refused; continue; }
            unsigned v = applied % 2u;
            apply_fields(v, &loaded, &authored, &defaulted);
            ++applied;
        }
    }
    printf("main_pass_programs=%u refused=%u authored_words=%u defaulted_pwm_words=%u\n",
           applied, refused, authored, defaulted);
    return !(applied == 640 && refused == 0 && authored == 4580 && defaulted == 2460);
}
