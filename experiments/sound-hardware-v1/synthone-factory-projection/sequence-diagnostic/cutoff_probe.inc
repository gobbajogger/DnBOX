/* SPDX-License-Identifier: GPL-3.0-only */
/* Host-only trial of original routes 1 and 2. Both clocks remain owned by effects. */
extern unsigned s1_cutoff_probe_route, s1_env_probe_route;

static void cutoff_probe_mix(s1_dry_part *part, float *left, unsigned frames, unsigned route)
{
    s1_trem_lfo a = part->lfo[0], b = part->lfo[1];
    const unsigned pitch = (unsigned)part->patch.pitch_lfo;
    const int pitch_active = ((pitch & 1u) && a.level != 0.f) || ((pitch & 2u) && b.level != 0.f);
    for (unsigned v = 0; v < S1_CORE_VOICES; ++v)
        s1_dry_begin_block(&part->voice[v]);
    for (unsigned i = 0; i < frames; ++i) {
        const float gain_a = lfo_step(&a);
        const float gain_b = lfo_step(&b);
        const float scale = route == 1u ? gain_a : route == 2u ? gain_b : 1.f;
        const float env_scale = s1_env_probe_route == 1u ? gain_a :
                                s1_env_probe_route == 2u ? gain_b : 1.f;
        const float pa = .5f * (1.f + a.smooth) * a.level;
        const float pb = .5f * (1.f + b.smooth) * b.level;
        const float pg = pitch == 1u ? pa : pitch == 2u ? pb : .5f * (pa + pb);
        const float ratio = 1.f + 0.0594630944f * pg;
        for (unsigned v = 0; v < S1_CORE_VOICES; ++v) {
            s1_dry_voice *voice = &part->voice[v];
            if (!voice->started) continue;
            const float base = voice->prepared.cutoff_hz;
            const float depth = voice->prepared.filter_depth;
            voice->prepared.cutoff_hz = base * scale;
            voice->prepared.filter_depth = depth * env_scale;
            if (pitch_active)
                s1_dry_mix_pitch(voice, left + i, &ratio, 1);
            else
                left[i] += s1_dry_sample(voice);
            voice->prepared.cutoff_hz = base;
            voice->prepared.filter_depth = depth;
        }
    }
}
