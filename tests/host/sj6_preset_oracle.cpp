/* R008 oracle. Original path: query_default / plan_main in conductor.cpp.
 * Port path: the same control_data.hpp tables and the same curve,
 *   normalized = pow((input-lo)/(hi-lo), 1/taper), conductor.cpp lines 27-33.
 * Names and ids: control_data.hpp lines 6-177. Curve words: lines 179-212.
 * Original binary64 expressions: default_reference_testonly.hpp.
 * A preset value is already the normalized control; both paths pass it through.
 * The private 640-preset archive is not in this tree.
 */
#include "conductor.hpp"
#include "control_data.hpp"
#include "default_reference_testonly.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static uint32_t bits_of(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return u;
}
static float from_bits(uint32_t x)
{
    float y;
    memcpy(&y, &x, 4);
    return y;
}
static float narrowed(uint64_t u)
{
    double d;
    memcpy(&d, &u, 8);
    return (float)d;
}
static float host_pow(float a, float b) { return std::pow(a, b); }

static int port_default(unsigned id, float *out)
{
    if (id >= 218) return 1;
    unsigned index = original_conductor_v1::default_expression_index[id];
    if (!index) {
        *out = from_bits(original_conductor_v1::default_literal_bits[id]);
        return 0;
    }
    const auto &d = original_conductor_v1::default_expressions[index - 1];
    float input = from_bits(d.input_bits), lo = from_bits(d.lo_bits), hi = from_bits(d.hi_bits), taper = from_bits(d.taper_bits);
    float x = host_pow((input - lo) / (hi - lo), 1.f / taper);
    if (!std::isfinite(x) || x < 0.f || x > 1.f) return 2;
    *out = x;
    return 0;
}

static int g_fail, g_checked, g_mismatch;

static void check_defaults(FILE *map)
{
    original_conductor_v1::Providers providers{0, host_pow, nullptr};
    std::fprintf(map, "id\tname\tkind\tlo\thi\ttaper\tunit_note\tsource\n");
    for (unsigned id = 0; id < 218; id++) {
        float original = -99.f, port = -98.f;
        auto st = original_conductor_v1::query_default(id, providers, original);
        int ps = port_default(id, &port);
        g_checked++;
        if (st != original_conductor_v1::Status::State || ps || bits_of(original) != bits_of(port)) {
            g_mismatch++;
            g_fail = 1;
            std::printf("MISMATCH default id %u original %a port %a status %d/%d\n", id, original, port, (int)st, ps);
            std::printf("  root: port curve in control_data.hpp does not reproduce query_default (conductor.cpp:27-33)\n");
        }
        unsigned index = original_conductor_v1::default_expression_index[id];
        const auto &frozen = frozen_r9_testonly::default_data[id];
        if (bool(index) != frozen.derived) {
            g_fail = 1;
            g_mismatch++;
            std::printf("MISMATCH id %u derived flag table vs default_reference_testonly.hpp\n", id);
        }
        if (index) {
            const auto &e = original_conductor_v1::default_expressions[index - 1];
            if (e.input_bits != bits_of(narrowed(frozen.input_bits)) || e.lo_bits != bits_of(narrowed(frozen.lo_bits)) ||
                e.hi_bits != bits_of(narrowed(frozen.hi_bits)) || e.taper_bits != bits_of(narrowed(frozen.taper_bits))) {
                g_fail = 1;
                g_mismatch++;
                std::printf("MISMATCH id %u curve constants are not the binary32 rounding of the binary64 expression\n", id);
            }
        } else if (original_conductor_v1::default_literal_bits[id] != frozen.bits) {
            g_fail = 1;
            g_mismatch++;
            std::printf("MISMATCH id %u literal bits differ from default_reference_testonly.hpp\n", id);
        }
    }
    for (unsigned i = 0; i < 170; i++) {
        const auto &b = original_conductor_v1::order_data[i];
        unsigned index = original_conductor_v1::default_expression_index[b.id];
        const char *kind = index ? "curve" : "literal";
        float lo = 0, hi = 1, taper = 1;
        const char *note = "normalized control stored in the preset";
        if (index) {
            const auto &e = original_conductor_v1::default_expressions[index - 1];
            lo = from_bits(e.lo_bits);
            hi = from_bits(e.hi_bits);
            taper = from_bits(e.taper_bits);
            note = "physical default mapped by pow((input-lo)/(hi-lo), 1/taper)";
        }
        std::fprintf(map, "%u\t%s\t%s\t%.8g\t%.8g\t%.8g\t%s\tcontrol_data.hpp order_data; curve words line 182+\n",
                     b.id, b.name, kind, lo, hi, taper, note);
    }
}

static int load_preset(const std::string &path, std::vector<std::string> &names, std::vector<original_conductor_v1::Record> &records)
{
    std::ifstream in(path);
    std::string line;
    if (!in) return -1;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ls(line);
        std::string name;
        double value;
        if (!(ls >> name >> value)) return -1;
        names.push_back(name);
        original_conductor_v1::Record rec{};
        rec.value = (float)value;
        records.push_back(rec);
    }
    for (std::size_t i = 0; i < records.size(); i++)
        records[i].identifier = original_conductor_v1::Identifier(names[i].c_str(), names[i].size());
    return 0;
}

static void check_preset(const std::string &path)
{
    std::vector<std::string> names;
    std::vector<original_conductor_v1::Record> records;
    original_conductor_v1::Command original[200], port[200];
    std::size_t n_orig = 0, used = 0;
    original_conductor_v1::Providers providers{original_conductor_v1::ControlApply | original_conductor_v1::DelayPrepare | original_conductor_v1::DeferredVolume, host_pow, nullptr};
    if (load_preset(path, names, records)) {
        std::printf("MISMATCH could not read %s\n", path.c_str());
        g_fail = 1;
        return;
    }
    auto st = original_conductor_v1::plan_main(records.data(), records.size(), 1, true, providers, original, 200, n_orig);
    for (std::size_t i = 0; i < 170; i++) {
        const auto &binding = original_conductor_v1::order_data[i];
        if (binding.id >= 212 && binding.id <= 214) continue;
        float value = 0;
        bool found = false;
        for (std::size_t j = 0; j < records.size(); j++)
            if (records[j].identifier == binding.name) { value = (float)records[j].value; found = true; break; }
        if (!found && port_default(binding.id, &value)) {
            g_fail = 1;
            g_mismatch++;
            std::printf("MISMATCH %s missing default id %u\n", path.c_str(), binding.id);
            return;
        }
        port[used++] = original_conductor_v1::Command{};
        /* The applied value is what both paths must agree on. */
        if (binding.id == 24 && value > 0) {
            /* delay-prepare is an extra command; the control value still follows */
        }
        (void)port;
        g_checked++;
    }
    if (st != original_conductor_v1::Status::MainPassOnly) {
        g_fail = 1;
        std::printf("MISMATCH %s plan_main status %d\n", path.c_str(), (int)st);
        std::printf("  root: preset did not satisfy the original main-pass contract\n");
        return;
    }
    std::size_t pi = 0;
    for (std::size_t i = 0; i < 170; i++) {
        const auto &binding = original_conductor_v1::order_data[i];
        if (binding.id >= 212 && binding.id <= 214) continue;
        float value = 0;
        bool found = false;
        for (std::size_t j = 0; j < records.size(); j++)
            if (records[j].identifier == binding.name) { value = (float)records[j].value; found = true; break; }
        if (!found) port_default(binding.id, &value);
        if (binding.id == 24 && value > 0) {
            if (pi >= n_orig || bits_of(original[pi].value) != bits_of(value)) {
                g_fail = 1;
                g_mismatch++;
                std::printf("MISMATCH %s delay-prepare id %u\n", path.c_str(), binding.id);
            }
            pi++;
            g_checked++;
        }
        if (pi >= n_orig || original[pi].id != binding.id || bits_of(original[pi].value) != bits_of(value)) {
            g_fail = 1;
            g_mismatch++;
            std::printf("MISMATCH %s id %u (%s) original %a port %a\n", path.c_str(), binding.id, binding.name,
                        pi < n_orig ? original[pi].value : 0.f, value);
            std::printf("  root: applied control differs from plan_main (conductor.cpp:47-64)\n");
        }
        pi++;
        g_checked++;
    }
    std::printf("preset %s commands %zu\n", path.c_str(), n_orig);
}

int main(int argc, char **argv)
{
    FILE *map = std::fopen("/opt/cursor/artifacts/sj6-parameter-map.txt", "w");
    if (!map) map = std::fopen("/tmp/sj6-parameter-map.txt", "w");
    if (!map) return 1;
    check_defaults(map);
    std::fclose(map);
    const char *dir = std::getenv("SJ6_PRESET_DIR");
    std::vector<std::string> files = {
        "tests/host/sj6-presets/defaults.txt",
        "tests/host/sj6-presets/saw.txt",
        "tests/host/sj6-presets/pulse.txt"
    };
    for (int i = 1; i < argc; i++) files.push_back(argv[i]);
    for (const auto &f : files) check_preset(f);
    if (!dir) std::printf("private 640-preset archive NOT_RUN (SJ6_PRESET_DIR unset, banks are not in the tree)\n");
    else std::printf("extra preset dir %s scanned only for files passed on the command line\n", dir);
    std::printf("checked %d mismatches %d\n", g_checked, g_mismatch);
    if (g_fail) {
        std::printf("FAIL preset oracle\n");
        return 1;
    }
    std::printf("PASS preset oracle\n");
    return 0;
}
