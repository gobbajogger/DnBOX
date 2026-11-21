/* SPDX-License-Identifier: GPL-3.0-only */
/* Included after the pinned 808 source so the reference owns its state. */
static drum808_t baseline;
void baseline_init(void) { drum808_init(&baseline); }
void baseline_trigger(int track, float velocity) { drum808_trigger(&baseline, track, velocity); }
void baseline_render(float *left, float *right, float *reverb, float *delay, int n)
{
    drum808_render_st(&baseline, left, right, reverb, delay, n);
}
