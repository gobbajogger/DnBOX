/* Focused dynamic ownership / block-partition API test against frozen C.
 * No algorithm copy, reference PCM, original engine or audio-file output.
 * Each logical owner is evaluated for 4096 frames in two orders: 8192 total
 * evaluated frames per owner, within the requested bound. */
#include "s1_dry_voice.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

enum { OWNERS = 2, FRAMES = 4096, MAX_BLOCKS = 256 };

typedef struct {
    unsigned ends[MAX_BLOCKS];
    unsigned count, requested_group_counts[3];
} block_plan;

typedef struct {
    float output[OWNERS][FRAMES];
    s1_dry_voice checkpoint[OWNERS][MAX_BLOCKS];
} expected_data;

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "ownership check failed at line %d: %s\n", __LINE__, #test); \
    return 1; } } while (0)

static uint32_t word(float x)
{ uint32_t bits; memcpy(&bits, &x, sizeof bits); return bits; }

static int finite_value(float x)
{ return (word(x) & 0x7f800000u) != 0x7f800000u; }

static unsigned off_frame(unsigned owner)
{ return owner == 0 ? 2049u : 3071u; }

static unsigned next_event(unsigned owner, unsigned frame)
{
    const unsigned events[2][5] = {
        {257u, 1027u, 2049u, 3007u, FRAMES},
        {131u, 777u, 1703u, 3071u, 3701u}
    };
    unsigned next = FRAMES;
    for (unsigned i = 0; i < 5; ++i)
        if (events[owner][i] > frame && events[owner][i] < next)
            next = events[owner][i];
    return next;
}

static int make_plan(unsigned owner, block_plan *plan)
{
    const unsigned groups[3] = {1u, 17u, 64u};
    unsigned frame = 0, group = owner == 0 ? 0u : 2u;
    memset(plan, 0, sizeof *plan);
    while (frame < FRAMES) {
        CHECK(plan->count < MAX_BLOCKS);
        unsigned size = groups[group];
        unsigned event = next_event(owner, frame);
        if (frame + size > event) size = event - frame;
        CHECK(size > 0 && size <= S1_DRY_MAX_BLOCK);
        frame += size;
        plan->ends[plan->count++] = frame;
        ++plan->requested_group_counts[group];
        group = (group + 1u) % 3u;
    }
    CHECK(frame == FRAMES);
    return 0;
}

static int initialize(unsigned owner, s1_dry_voice *voice, s1_dry_controls *p)
{
    s1_dry_reset(voice);
    s1_dry_frozen_controls(p);
    if (owner == 0) {
        p->amp = (s1_dry_adsr){0.003f, 0.03f, 0.55f, 0.02f};
        p->filter = (s1_dry_adsr){0.02f, 0.1f, 0.4f, 0.03f};
        p->cutoff_hz = 2000.0f; p->resonance = 0.9f; p->filter_depth = 0.5f;
    } else {
        p->amp = (s1_dry_adsr){0.001f, 0.07f, 0.3f, 0.004f};
        p->filter = (s1_dry_adsr){0.004f, 0.02f, 0.65f, 0.05f};
        p->cutoff_hz = 8000.0f; p->resonance = 0.25f; p->filter_depth = 1.2f;
        p->oscillator1_level = 0.8f; p->balance = 0.2f;
    }
    CHECK(s1_dry_configure(voice, p) == S1_DRY_OK);
    CHECK(s1_dry_start(voice, owner == 0 ? 261.6255798339844f : 1046.5023193359375f,
                        owner == 0 ? 100u : 20u, owner == 0 ? 0.0f : 0.375f) == S1_DRY_OK);
    return 0;
}

/* Controls and gate changes apply before the numbered output frame. The two
 * owners have different event timelines; note-off coincides with a physical
 * edit in each timeline and occurs at a different frame for each owner. */
static int apply_events(unsigned owner, unsigned frame, s1_dry_voice *voice,
                        s1_dry_controls *p, unsigned *edits, unsigned *offs)
{
    int changed = 1;
    if (owner == 0) {
        switch (frame) {
        case 257:
            p->cutoff_hz = 64.0f; p->resonance = 0.98f; p->filter_depth = 1.2f;
            p->filter.sustain = 0.2f; p->amp.sustain = 0.4f;
            break;
        case 1027:
            p->cutoff_hz = 22050.0f; p->resonance = 0.9f; p->filter_depth = 0.5f;
            p->amp.decay_s = 0.2f;
            break;
        case 2049:
            p->cutoff_hz = 500.0f; p->resonance = 0.25f; p->filter_depth = 1.2f;
            p->amp.release_s = 0.004f;
            break;
        case 3007:
            p->cutoff_hz = 8000.0f; p->resonance = 0.5f; p->filter_depth = 0.2f;
            p->amp.release_s = 0.2f; p->filter.release_s = 0.0f;
            break;
        default: changed = 0; break;
        }
    } else {
        switch (frame) {
        case 131:
            p->cutoff_hz = 22050.0f; p->resonance = 0.5f; p->filter_depth = 1.2f;
            p->filter.sustain = 0.75f;
            break;
        case 777:
            p->cutoff_hz = 500.0f; p->resonance = 0.0f; p->filter_depth = 0.0f;
            p->amp.sustain = 0.55f; p->filter.attack_s = 0.2f;
            break;
        case 1703:
            p->cutoff_hz = 2000.0f; p->resonance = 0.98f; p->filter_depth = 1.2f;
            p->oscillator1_level = 0.65f; p->balance = 0.1f;
            break;
        case 3071:
            p->cutoff_hz = 8000.0f; p->resonance = 0.9f; p->filter_depth = 0.75f;
            p->amp.release_s = 0.05f; p->filter.release_s = 0.2f;
            break;
        case 3701:
            p->cutoff_hz = 64.0f; p->resonance = 0.98f; p->filter_depth = 1.2f;
            p->amp.release_s = 0.004f; p->filter.release_s = 0.0f;
            break;
        default: changed = 0; break;
        }
    }
    if (changed) {
        CHECK(s1_dry_configure(voice, p) == S1_DRY_OK);
        ++*edits;
    }
    if (frame == off_frame(owner)) {
        CHECK(s1_dry_note_off(voice) == S1_DRY_OK);
        ++*offs;
    }
    return 0;
}

int main(void)
{
    block_plan plan[OWNERS];
    expected_data expected;
    unsigned independent_edits[OWNERS] = {0, 0}, independent_offs[OWNERS] = {0, 0};
    unsigned finite_samples = 0;
    for (unsigned owner = 0; owner < OWNERS; ++owner) {
        CHECK(make_plan(owner, &plan[owner]) == 0);
        s1_dry_voice voice;
        s1_dry_controls controls;
        CHECK(initialize(owner, &voice, &controls) == 0);
        unsigned checkpoint = 0;
        /* Independent owner baselines use single-frame API calls throughout. */
        for (unsigned frame = 0; frame < FRAMES; ++frame) {
            CHECK(apply_events(owner, frame, &voice, &controls,
                &independent_edits[owner], &independent_offs[owner]) == 0);
            CHECK(s1_dry_render(&voice, &expected.output[owner][frame], 1) == S1_DRY_OK);
            CHECK(finite_value(expected.output[owner][frame]));
            ++finite_samples;
            if (frame + 1u == plan[owner].ends[checkpoint])
                expected.checkpoint[owner][checkpoint++] = voice;
        }
        CHECK(checkpoint == plan[owner].count);
    }

    s1_dry_voice voice[OWNERS];
    s1_dry_controls controls[OWNERS];
    for (unsigned owner = 0; owner < OWNERS; ++owner)
        CHECK(initialize(owner, &voice[owner], &controls[owner]) == 0);
    unsigned position[OWNERS] = {0, 0}, next_block[OWNERS] = {0, 0};
    unsigned interleaved_edits[OWNERS] = {0, 0}, interleaved_offs[OWNERS] = {0, 0};
    unsigned full_state_checks = 0, untouched_checks = 0, sample_matches = 0;
    unsigned turn = 0;
    const unsigned order[8] = {0, 1, 1, 0, 1, 0, 0, 1};
    while (position[0] < FRAMES || position[1] < FRAMES) {
        unsigned owner = order[turn++ % 8u];
        if (position[owner] == FRAMES) owner = 1u - owner;
        unsigned other = 1u - owner;
        CHECK(position[owner] < FRAMES && next_block[owner] < plan[owner].count);
        s1_dry_voice untouched = voice[other];
        s1_dry_controls untouched_controls = controls[other];
        CHECK(apply_events(owner, position[owner], &voice[owner], &controls[owner],
            &interleaved_edits[owner], &interleaved_offs[owner]) == 0);
        unsigned end = plan[owner].ends[next_block[owner]];
        unsigned frames = end - position[owner];
        float output[S1_DRY_MAX_BLOCK];
        CHECK(s1_dry_render(&voice[owner], output, frames) == S1_DRY_OK);
        for (unsigned i = 0; i < frames; ++i) {
            CHECK(finite_value(output[i]));
            ++finite_samples;
            if (word(output[i]) != word(expected.output[owner][position[owner] + i])) {
                fprintf(stderr, "owner %u frame %u: interleaved=%08" PRIx32
                    " independent=%08" PRIx32 "\n", owner, position[owner] + i,
                    word(output[i]), word(expected.output[owner][position[owner] + i]));
                return 1;
            }
            ++sample_matches;
        }
        if (memcmp(&voice[owner], &expected.checkpoint[owner][next_block[owner]],
                   sizeof(s1_dry_voice)) != 0) {
            fprintf(stderr, "owner %u full state differs after frame %u\n", owner, end);
            return 1;
        }
        ++full_state_checks;
        CHECK(memcmp(&voice[other], &untouched, sizeof untouched) == 0);
        CHECK(memcmp(&controls[other], &untouched_controls, sizeof untouched_controls) == 0);
        ++untouched_checks;
        position[owner] = end;
        ++next_block[owner];
    }
    for (unsigned owner = 0; owner < OWNERS; ++owner) {
        CHECK(position[owner] == FRAMES && next_block[owner] == plan[owner].count);
        CHECK(independent_edits[owner] == interleaved_edits[owner]);
        CHECK(independent_offs[owner] == 1 && interleaved_offs[owner] == 1);
        CHECK(voice[owner].gate == 0);
        for (unsigned group = 0; group < 3; ++group)
            CHECK(plan[owner].requested_group_counts[group] > 0);
    }
    CHECK(independent_edits[0] == 4 && independent_edits[1] == 5);
    CHECK(sample_matches == OWNERS * FRAMES);
    CHECK(finite_samples == 2u * OWNERS * FRAMES);
    printf("{\"status\":\"PASS\",\"logical_owners\":2,\"frames_per_execution_per_owner\":4096,"
           "\"executions_per_owner\":2,\"total_evaluated_frames_per_logical_owner\":8192,"
           "\"compared_sample_words\":%u,\"finite_output_checks\":%u,"
           "\"full_state_checkpoint_checks\":%u,\"untouched_owner_checks\":%u,"
           "\"block_groups\":[1,17,64],\"owner_block_counts\":[%u,%u],"
           "\"live_control_edits_per_owner\":[4,5],\"note_off_frames\":[2049,3071],"
           "\"maximum_requested_filter_depth\":1.2,\"voice_state_bytes_compared\":%zu,"
           "\"candidate_source_modified\":false,\"reference_pcm_read\":false,"
           "\"audio_files_written\":false,\"scope\":\"dynamic ownership and block-partition API test; no fidelity gate\"}\n",
           sample_matches, finite_samples, full_state_checks, untouched_checks,
           plan[0].count, plan[1].count, sizeof(s1_dry_voice));
    return 0;
}
