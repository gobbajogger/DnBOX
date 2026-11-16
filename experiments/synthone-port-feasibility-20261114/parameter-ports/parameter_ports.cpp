#include "parameter_ports.hpp"
#include "float_math.h"
#if defined(S1_NO_DOUBLE)
#include "exact.h"
#endif
#include <math.h>
#include <stdint.h>

namespace synthone_ports {
namespace {
constexpr float minimum_half_time = .000001f;
constexpr float maximum_half_time = .99f;
// Covers every current physical metadata range (largest maximum is 48000).
constexpr float maximum_target_magnitude = 48000.f;
bool valid_target(float value) {
    return s1_finitef(value) && value >= -maximum_target_magnitude && value <= maximum_target_magnitude;
}
bool valid_rate(int rate) { return rate == 44100 || rate == 48000 || rate == 96000; }
float clamp_half_time(float value) {
    return value < minimum_half_time ? minimum_half_time : value > maximum_half_time ? maximum_half_time : value;
}
bool span(const void *pointer, size_t bytes, uintptr_t &begin, uintptr_t &end) {
    if (!pointer) return false;
    begin = reinterpret_cast<uintptr_t>(pointer);
    if (begin > UINTPTR_MAX - bytes) return false;
    end = begin + bytes;
    return true;
}
bool overlaps(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d) { return a < d && c < b; }
} // namespace

bool Ports::valid_values(const float *values, size_t count, float *outputs, size_t output_count) const {
    if (count != parameter_count || output_count != parameter_count) return false;
    uintptr_t ib, ie, ob, oe, sb, se;
    if (!span(values, parameter_count * sizeof(float), ib, ie) ||
        !span(outputs, parameter_count * sizeof(float), ob, oe) ||
        !span(this, sizeof *this, sb, se)) return false;
    if (overlaps(ib, ie, sb, se) || overlaps(ob, oe, sb, se) ||
        (values != outputs && overlaps(ib, ie, ob, oe))) return false;
    for (unsigned i = 0; i < parameter_count; ++i) if (!valid_target(values[i])) return false;
    return true;
}

bool Ports::init(float *outputs, size_t output_count, const float *values,
                 size_t value_count, int sample_rate) {
    if (!valid_rate(sample_rate) || !valid_values(values, value_count, outputs, output_count)) return false;
    outputs_ = outputs;
    common_ = {};
    apply_restore(values, sample_rate);
    return true;
}

void Ports::apply_restore(const float *values, int sample_rate) {
    // Source sp_port_init retains c1/c2, clears yt1 and makes coefficients dirty.
    common_.sr = sample_rate;
#if defined(S1_NO_DOUBLE)
    common_.onedsr = s1_port_onedsr(common_.sr);
    common_.prvhtim = -100.f;
#else
    common_.onedsr = 1.0 / common_.sr;
    common_.prvhtim = -100.0;
#endif
    common_.htime = clamp_half_time(values[half_time_parameter]);
    for (unsigned i = 0; i < parameter_count; ++i) {
        const auto slot = selected_slots[i];
        if (slot != unsmoothed_slot) {
            selected_[slot].target = values[i];
            selected_[slot].yt1 = 0;
        }
        outputs_[i] = values[i];
    }
}

bool Ports::restore(const float *values, size_t value_count, int sample_rate) {
    if (!outputs_ || !valid_rate(sample_rate) ||
        !valid_values(values, value_count, outputs_, parameter_count)) return false;
    apply_restore(values, sample_rate);
    return true;
}

bool Ports::update_half_time(float half_time) {
    if (!outputs_ || !s1_finitef(half_time) ||
        half_time < minimum_half_time || half_time > maximum_half_time) return false;
    common_.htime = half_time;
    return true;
}

bool Ports::set_resolved_target(unsigned parameter, float value) {
    if (!outputs_ || parameter >= parameter_count || !valid_target(value)) return false;
    if (parameter == half_time_parameter && !s1_finitef(outputs_[parameter])) return false;
    const auto slot = selected_slots[parameter];
    if (slot == unsmoothed_slot) outputs_[parameter] = value;
    else selected_[slot].target = value;
    // _setSynthParameterHelper uses getParameter (current output), not target.
    if (parameter == half_time_parameter) common_.htime = clamp_half_time(outputs_[parameter]);
    return true;
}

bool Ports::tick() {
    if (!outputs_) return false;
    if (common_.prvhtim != common_.htime) {
#if defined(S1_NO_DOUBLE)
        s1_port_coeffs(common_.onedsr, common_.htime, &common_.c1, &common_.c2);
#else
        common_.c2 = pow(0.5, common_.onedsr / common_.htime);
        common_.c1 = 1.0 - common_.c2;
#endif
        common_.prvhtim = common_.htime;
    }
    for (unsigned slot = 0; slot < selected_count; ++slot) {
        auto &state = selected_[slot];
        outputs_[selected_indices[slot]] = state.yt1 = common_.c1 * state.target + common_.c2 * state.yt1;
    }
    return true;
}

TargetView Ports::target(unsigned parameter) const {
    if (!outputs_ || parameter >= parameter_count) return {false, 0};
    const auto slot = selected_slots[parameter];
    return {true, slot == unsmoothed_slot ? outputs_[parameter] : selected_[slot].target};
}

PortView Ports::port(unsigned parameter) const {
    PortView view{};
    if (!outputs_ || parameter >= parameter_count || selected_slots[parameter] == unsmoothed_slot) return view;
    view.valid = true;
    view.state.htime = common_.htime; view.state.c1 = common_.c1; view.state.c2 = common_.c2;
    view.state.yt1 = selected_[selected_slots[parameter]].yt1;
    view.state.prvhtim = common_.prvhtim; view.state.sr = common_.sr; view.state.onedsr = common_.onedsr;
    return view;
}

void Ports::teardown() {
    outputs_ = nullptr;
    common_ = {};
    for (auto &state : selected_) state = {};
}
} // namespace synthone_ports
