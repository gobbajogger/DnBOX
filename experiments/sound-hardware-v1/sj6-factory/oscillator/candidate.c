/* Test-only observation immediately after the actual voice oscillator and
 * before the sub source, envelope, filter, gain and stereo mix. */
#include "juno6.h"
#include <stddef.h>
#include <string.h>

static float *output;
static size_t capacity;
static size_t written;
static unsigned selected_layer;
static unsigned sounding_paths;

void observe_oscillator(unsigned voice, unsigned layer, float x)
{
    if (!output) return;
    if (voice < 2 && layer < 2 && x != 0.f)
        sounding_paths |= 1u << (voice * 2 + layer);
    if (voice == 0 && layer == selected_layer) {
        if (written < capacity) output[written] = x;
        ++written;
    }
}

int candidate_oscillator(unsigned layer, float selector, float amount, float enable,
                         unsigned note, float *increment, float *samples, size_t count,
                         unsigned add_peers)
{
    if (layer >= DNBX_LAYERS || note > 127 || !samples || !count) return 0;
    juno6_t part;
    juno6_init(&part, 44100.f);
    juno6_oscillator_edit edit = {selector, amount, enable};
    juno6_apply_pre_note_wave(&part, layer, &edit);
    const unsigned ids[] = {82 + layer, 78 + layer, 210 + layer};
    const float expected[] = {selector, amount, enable};
    for (unsigned i = 0; i < 3; ++i) {
        float raw;
        if (dnbx_read_raw(&part.core, ids[i], &raw) != DNBX_OK ||
            memcmp(&raw, &expected[i], sizeof raw) != 0) return 0;
    }
    if (add_peers) {
        juno6_oscillator_edit other = {1.f, .78125f, 1.f};
        unsigned peer = layer ^ 1u;
        juno6_apply_pre_note_wave(&part, peer, &other);
        float raw;
        if (dnbx_read_raw(&part.core, 78 + peer, &raw) != DNBX_OK || raw != other.amount) return 0;
    }
    if (!juno6_note_on(&part, note, 127)) return 0;
    if (add_peers && !juno6_note_on(&part, (note + 19) % 128, 73)) return 0;
    *increment = part.core.voice[0].layer[layer].increment;
    output = samples;
    capacity = count;
    written = 0;
    selected_layer = layer;
    sounding_paths = 0;
    for (size_t done = 0; done < count;) {
        float left[128], right[128];
        unsigned frames = count - done < 128 ? (unsigned)(count - done) : 128;
        if (!juno6_render_stereo(&part, left, right, frames)) return 0;
        done += frames;
    }
    output = NULL;
    return written == count && (!add_peers || sounding_paths == 15u);
}

/* Exercise both legacy quality modes, including a normal macro edit after the
 * selection, through the complete stereo core. The baseline provides the hash. */
int stock_stereo(unsigned mode, float *samples, size_t count)
{
    juno6_t part;
    if (mode > DNBX_OSC_RAW || !count) return 0;
    juno6_init(&part, 44100.f);
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        dnbx_layer_params p;
        if (dnbx_get_layer(&part.core, layer, &p) != DNBX_OK) return 0;
        p.oscillator_mode = mode;
        if (dnbx_set_layer(&part.core, layer, &p) != DNBX_OK) return 0;
    }
    if (!juno6_set(&part, 0, .28f) || !juno6_note_on(&part, 64, 111)) return 0;
    return juno6_render_stereo(&part, samples, samples + count, (unsigned)count);
}
