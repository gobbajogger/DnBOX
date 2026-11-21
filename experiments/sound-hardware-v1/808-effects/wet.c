/* SPDX-License-Identifier: GPL-3.0-only */
#include "sc808_engine.h"
#include "fxbus.h"
#include "drum808.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { RATE = 44100, FRAMES = 88200, BLOCK = 128, DELAY_WORDS = 79200 };
static int16_t delay_memory[DELAY_WORDS];
static fxbus_t bus;
static drum808_t kit;
static fxbus_t kit_verb;

static void set(sc808_engine_t *s, const char *name, unsigned value)
{
    char text[16];
    snprintf(text, sizeof text, "%u", value);
    if (!sc808_set_param(s, name, text)) exit(3);
}

static void controls(sc808_engine_t *s, int rv, int dl, int dist, int glue)
{
    set(s, "volume", 127);
    set(s, "sd_rev", rv ? 127 : 0);
    set(s, "sd_dly", dl ? 127 : 0);
    set(s, "rev_level", 93);
    set(s, "rev_decay", 70);
    set(s, "rev_tone", 60);
    set(s, "rev_hpf", 55);
    set(s, "dly_level", 93);
    set(s, "dly_time", 5);
    set(s, "dly_fdbk", 68);
    set(s, "dly_tone", 60);
    set(s, "dly_hpf", 55);
    set(s, "master_dist", (unsigned)dist);
    set(s, "master_drive", 64);
    set(s, "comp", (unsigned)glue);
}

int main(int argc, char **argv)
{
    if (argc != 9) return 2;
    int kit_reverb = atoi(argv[8]);
    int kit_glue = atoi(argv[7]);
    int kit_master = atoi(argv[6]);
    int rv=atoi(argv[1]), dl=atoi(argv[2]), dist=atoi(argv[3]), glue=atoi(argv[4]);
    sc808_engine_t *dry = sc808_create(RATE), *wet = sc808_create(RATE);
    if (!dry || !wet) return 2;
    controls(dry, 0, 0, 0, 0);
    controls(wet, rv, dl, dist, glue);
    drum808_init(&kit);
    drum808_set(&kit, D8_KIT, 0, 127);
    drum808_set(&kit, D8_KIT, D8_KIT_DIST, kit_master ? dist : 0);
    drum808_set(&kit, D8_KIT, D8_KIT_DRIVE, 64);
    drum808_set(&kit, D8_KIT, D8_KIT_GLUE, kit_glue ? glue : 0);
    drum808_set(&kit, D8_KIT, D8_KIT_RV_BUS, kit_reverb);
    fxbus_init_808_reverb(&kit_verb);
    fxbus_set(&kit_verb, FX_RV_LEVEL, 93);
    fxbus_set(&kit_verb, FX_RV_DECAY, 70);
    fxbus_set(&kit_verb, FX_RV_TONE, 60);
    fxbus_set(&kit_verb, FX_RV_HPF, 55);
    fxbus_init_packed(&bus, delay_memory, DELAY_WORDS);
    fxbus_set(&bus, FX_VOLUME, 127);
    fxbus_set(&bus, FX_RV_LEVEL, 93);
    fxbus_set(&bus, FX_RV_DECAY, 70);
    fxbus_set(&bus, FX_RV_TONE, 60);
    fxbus_set(&bus, FX_RV_HPF, 55);
    fxbus_set(&bus, FX_DL_LEVEL, 93);
    fxbus_set(&bus, FX_DL_TIME, 5);
    fxbus_set(&bus, FX_DL_FDBK, 68);
    fxbus_set(&bus, FX_DL_TONE, 60);
    fxbus_set(&bus, FX_DL_HPF, 55);
    fxbus_set(&bus, FX_DRIVE, 64);
    fxbus_set(&bus, FX_DIST, kit_master ? 0 : dist);
    fxbus_set(&bus, FX_COMP, kit_glue ? 0 : glue);
    FILE *file = fopen(argv[5], "wbx");
    if (!file) return 2;

    for (int offset = 0; offset < FRAMES; offset += BLOCK) {
        int n = FRAMES-offset < BLOCK ? FRAMES-offset : BLOCK;
        float x[BLOCK], reference[BLOCK], left[BLOCK], right[BLOCK];
        if (offset == 0 || offset == 3072 || offset == 12288 || offset == 20480) {
            sc808_trigger(dry, SC808_SD, 118);
            sc808_trigger(wet, SC808_SD, 118);
        }
        sc808_render(dry, x, n);
        sc808_render(wet, reference, n);
        /* One post-fader snare at unity volume and unity selected sends.
         * Its original dry signal is also the candidate's exact FX input. */
        float kl[BLOCK], kr[BLOCK], send_r[BLOCK], send_d[BLOCK];
        for (int i = 0; i < n; ++i) {
            kl[i] = kr[i] = x[i];
            send_r[i] = rv ? x[i] : 0.0f;
            send_d[i] = dl ? x[i] : 0.0f;
        }
        if (kit_reverb) {
            fxbus_808_reverb(&kit_verb, send_r, kl, kr, n, 1.0f);
            memset(send_r, 0, (size_t)n * sizeof send_r[0]);
        }
        drum808_kit_process(&kit, kl, kr, send_r, send_d, n);
        fxbus_process_st(&bus, kl, kr, send_r, send_d, left, right, n);
        for (int i=0; i<n; ++i) {
            const float values[] = {x[i], reference[i], left[i], right[i]};
            for (unsigned j=0; j<4; ++j) if (!isfinite(values[j])) return 4;
            if (fwrite(values, sizeof values, 1, file) != 1) return 5;
        }
    }
    printf("drive at position 64: %.9g on 8W8, %.9g on DnBOX\n", 10.*64./127., (double)(kit_master ? kit.master.drive : bus.drive));
    sc808_destroy(wet);
    sc808_destroy(dry);
    return fclose(file) != 0;
}
