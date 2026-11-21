/* SPDX-License-Identifier: GPL-3.0-only */
/* The plugin and firmware receive the same decoded loop at unity rate. */
#include "plugin_api_v1.h"
#include "breaks.h"
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

extern plugin_api_v2_t *move_plugin_init_v2(const host_api_v1_t *host);

/* The original instance sets up device sample-browser symlinks at startup.
 * Unit tests supply an absolute WAV path, and need no device filesystem. */
int __wrap_mkdir(const char *path, mode_t mode) { (void)path; (void)mode; errno = EROFS; return -1; }
int __wrap_symlink(const char *target, const char *path) { (void)target; (void)path; errno = EROFS; return -1; }
int __wrap_unlink(const char *path) { (void)path; errno = EROFS; return -1; }

static int clock_state = MOVE_CLOCK_STATUS_STOPPED;
static float tempo;
static int get_clock(void) { return clock_state; }
static float get_bpm(void) { return tempo; }

int main(int argc, char **argv)
{
    if (argc != 8) return 2;
    unsigned samples = (unsigned)strtoul(argv[1], NULL, 10);
    unsigned frames = (unsigned)strtoul(argv[2], NULL, 10);
    unsigned slice = (unsigned)strtoul(argv[7], NULL, 10);
    if (slice >= 8 || samples < 64 || frames == 0 || frames >= samples / 8) return 2;
    unsigned bytes = (samples + 1) / 2;
    uint8_t *encoded = malloc(bytes);
    FILE *source = fopen(argv[3], "rb");
    if (!encoded || !source || fread(encoded, 1, bytes, source) != bytes ||
        fgetc(source) != EOF || fclose(source) != 0) return 3;
    FILE *out = fopen(argv[5], "wbx");
    if (!out) return 3;

    /* BB Gen divides the loop into eight integer-length slices. DnBOX uses
     * the exact samples per bar. Give each its native integer-unity-rate
     * transport setting. Both play the same source onset without a seek. */
    tempo = 44100.f * 60.f / (2.f * (float)(samples / 8));
    const host_api_v1_t host = {.api_version=1, .sample_rate=44100, .frames_per_block=128,
                                .get_bpm=get_bpm, .get_clock_status=get_clock};
    plugin_api_v2_t *api = move_plugin_init_v2(&host);
    void *plugin = api->create_instance(argv[6], NULL);
    if (!plugin) return 4;
    api->set_param(plugin, "A_sample_path", argv[4]);
    api->set_param(plugin, "complexity", "0");
    int16_t stereo[256];
    api->render_block(plugin, stereo, 128);
    clock_state = MOVE_CLOCK_STATUS_RUNNING;

    breaks_t part;
    breaks_init(&part);
    brk_loop_t loop = {.adpcm=encoded, .nsamples=samples, .rate=44100, .bars=1, .slices=8};
    breaks_set_loop(&part, 0, &loop);
    breaks_set(&part, BRK_COMPLEX, 0);
    breaks_set(&part, BRK_LEVEL, 127);
    breaks_step(&part, 0, 0, (float)samples / 16.f, 1);
    float mono[128];
    if (slice) {
        /* Commit transport start before sending a manual slice selection. */
        api->render_block(plugin, stereo, 1);
        breaks_render(&part, mono, 1);
        const uint8_t note[3] = {0x90, (uint8_t)(36 + slice), 127};
        api->on_midi(plugin, note, 3, MOVE_MIDI_SOURCE_EXTERNAL);
        breaks_slice_on(&part, (int)slice);
    }
    while (frames) {
        unsigned n = frames < 128 ? frames : 128;
        api->render_block(plugin, stereo, (int)n);
        breaks_render(&part, mono, (int)n);
        for (unsigned i = 0; i < n; ++i) {
            if (stereo[i * 2] != stereo[i * 2 + 1] || !isfinite(mono[i])) return 5;
            /* Use the original plugin's signed PCM16 output scale for both. */
            int16_t pair[2] = {stereo[i * 2], (int16_t)(mono[i] * 32767.f)};
            if (fwrite(pair, sizeof pair, 1, out) != 1) return 6;
        }
        frames -= n;
    }
    api->destroy_instance(plugin);
    free(encoded);
    return fclose(out) != 0;
}
