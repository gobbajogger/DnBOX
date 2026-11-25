#!/usr/bin/env python3
"""Port the existing authenticated b7-9 clock in the guarded joined source."""
import hashlib
import pathlib
import sys

p = pathlib.Path(sys.argv[1]).resolve()
local = pathlib.Path(__file__).resolve().parent
app = p / "firmware/src/app"
check_only = "--check" in sys.argv[2:] or "--guard-only" in sys.argv[2:]
inputs = {}

def load(name):
    path = app / name
    data = path.read_text()
    inputs[path] = data
    return data

def once(text, before, after):
    if text.count(before) != 1:
        raise SystemExit(f"clock preimage mismatch: {before!r}")
    return text.replace(before, after, 1)

trial = load("typed_controls_trial.h")
trial = once(trial, "    X0X_S1_FENCED_CLOCK ||",
    "    (X0X_S1_FENCED_CLOCK && !X0X_S1_CLOCK_NATIVE) ||")
# Enforce that the numeric port cannot silently activate in another profile.
trial = once(trial, "#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n",
    "#if X0X_S1_CLOCK_NATIVE && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || !X0X_S1_FENCED_CLOCK)\n"
    "#error Native clock requires the explicit typed trial and b7-9 feature flags\n"
    "#endif\n"
    "#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n")

owner = load("sj6_137_host_witness.h")
owner = once(owner, "#if X0X_S1_FENCED_CLOCK\n  if(!s1_clock_valid(b+S1_ARP_CLOCK_BYTE)) return 0;\n#endif",
    "#if X0X_S1_FENCED_CLOCK\n  if(!s1_clock_valid(b+S1_ARP_CLOCK_BYTE)) return 0;\n"
    "#if X0X_S1_CLOCK_NATIVE\n"
    "  if(b[S1_ARP_CLOCK_BYTE+2u] &&\n"
    "     (s->v[T_303][v][29]!=ORIGINAL_SOUND_SYNTHONE || original_sound_s1_program(s->v[T_303][v]))) return 0;\n"
    "#endif\n#endif")

rh = load("original_runtime.h")
rh = once(rh, """    double source_phase, source_delta;
    float source_factor, source_bpm;
    uint8_t source_stop;
    uint8_t source_set, source_wait;""", """#if X0X_S1_CLOCK_NATIVE
    uint64_t source_phase, source_delta, source_boundary;
    uint8_t source_stop, source_set, source_wait, source_bpm;
#else
    double source_phase, source_delta;
    float source_factor, source_bpm;
    uint8_t source_stop;
    uint8_t source_set, source_wait;
#endif""")

rt = load("original_runtime.c")
needle = "#if X0X_S1_FENCED_CLOCK\nint original_runtime_s1_clock"
rt = once(rt, needle, """#if X0X_S1_FENCED_CLOCK
#if X0X_S1_CLOCK_NATIVE
#include "s1_clock_bits.h"
static uint32_t source_factor_bits(unsigned i)
{
    /* Exact binary32 values of original units[i] / 192.f. */
    static const uint32_t f[] = {
        0x41000000,0x40c00000,0x40800000,0x40400000,0x40000000,
        0x3f800000,0x3f2aaaab,0x3f000000,0x3eaaaaab,0x3e800000,
        0x3e2aaaab,0x3e000000,0x3daaaaab,0x3d800000,0x3d2aaaab,
        0x3d000000,0x3caaaaab,0x3c800000,0x3c2aaaab
    };
    return f[i];
}
static uint32_t source_clock_rearm(original_runtime_arp *a, uint32_t *visible)
{
    uint32_t f = source_factor_bits(a->source_stop);
    uint32_t k = s1_clock_step(a->source_phase, f, visible);
    a->source_boundary = s1_clock_boundary(k + 1u, f);
    return k;
}
#endif
int original_runtime_s1_clock""")
rt = once(rt, """    static const uint16_t units[]={1536,1152,768,576,384,192,128,96,64,48,32,24,16,12,8,6,4,3,2};""", """#if !X0X_S1_CLOCK_NATIVE
    static const uint16_t units[]={1536,1152,768,576,384,192,128,96,64,48,32,24,16,12,8,6,4,3,2};
#endif""")
rt = once(rt, "factor>=sizeof units/sizeof units[0]", "factor>=19u")
rt = once(rt, """    a->source_set=1; a->source_bpm=bpm; a->source_stop=(uint8_t)factor;
    a->source_delta=(bpm/60.f)/44100.;
    a->source_factor=units[factor]/192.f;""", """#if X0X_S1_CLOCK_NATIVE
    /* The typed native editor/storage supports the complete original 1..200
     * integer panel, not the separate fractional Ableton Link input. */
    unsigned ibpm = (unsigned)bpm;
    if ((float)ibpm != bpm) return 0;
    a->source_set=1; a->source_bpm=(uint8_t)ibpm; a->source_stop=(uint8_t)factor;
    a->source_delta=s1_clock_delta_bits(bpm/60.f);
    (void)source_clock_rearm(a, 0);
#else
    a->source_set=1; a->source_bpm=bpm; a->source_stop=(uint8_t)factor;
    a->source_delta=(bpm/60.f)/44100.;
    a->source_factor=units[factor]/192.f;
#endif""")
# The first typed owner edit must not mutate waiting state if a future native
# caller supplies fractional BPM.
rt = once(rt, """    if (!a->source_set) a->source_wait=!!a->nheld;
#if X0X_S1_CLOCK_NATIVE""", """#if X0X_S1_CLOCK_NATIVE""")
rt = once(rt, """    if ((float)ibpm != bpm) return 0;
    a->source_set=1;""", """    if ((float)ibpm != bpm) return 0;
    if (!a->source_set) a->source_wait=!!a->nheld;
    a->source_set=1;""")
rt = once(rt, """#else
    a->source_set=1; a->source_bpm=bpm;""", """#else
    if (!a->source_set) a->source_wait=!!a->nheld;
    a->source_set=1; a->source_bpm=bpm;""")
rt = once(rt, """    if (p.reset) a->source_phase=0;
    if (on || (a->seq.sequencerLastNotesCount && !p.reset)) {""", """    if (p.reset) {
        a->source_phase=0;
#if X0X_S1_CLOCK_NATIVE
        a->source_boundary=0; /* next sample initializes exact boundary */
#endif
    }
    if (on || (a->seq.sequencerLastNotesCount && !p.reset)) {
#if X0X_S1_CLOCK_NATIVE
        if (!a->source_boundary) (void)source_clock_rearm(a, 0);
        a->source_phase = s1_clock_add(a->source_phase, a->source_delta);
        const int crossed = a->source_phase >= a->source_boundary;
        p.trigger = first || crossed;
        if (p.trigger && on && a->nheld) {
            uint32_t k;
            (void)source_clock_rearm(a, &k);
            if (k >= S1_CINTMAX) p.trigger=0;
            else p.index=(int)k;
        } else if (crossed) {
            (void)source_clock_rearm(a, 0);
        }
#else""")
rt = once(rt, """            else p.index=(int)i;
        }
    }
    return p;""", """            else p.index=(int)i;
        }
#endif
    }
    return p;""")
rt = once(rt, """            a->source_wait=0; a->source_phase=0; a->prm.clocked=0;""", """            a->source_wait=0; a->source_phase=0; a->prm.clocked=0;
#if X0X_S1_CLOCK_NATIVE
            a->source_boundary=0;
#endif""")
presets = load("original_presets.inc")
presets = once(presets, "        part->arp.source_phase=0;", """        part->arp.source_phase=0;
#if X0X_S1_CLOCK_NATIVE
        part->arp.source_boundary=0;
#endif""")
edits = {"typed_controls_trial.h":trial, "sj6_137_host_witness.h":owner,
         "original_runtime.h":rh, "original_runtime.c":rt, "original_presets.inc":presets}
dst = app / "s1_clock_bits.h"
if dst.exists():
    raise SystemExit("clock preimage mismatch: added s1_clock_bits.h exists")
if check_only:
    for f in inputs:
        print(hashlib.sha256(f.read_bytes()).hexdigest(), f.relative_to(p))
    raise SystemExit(0)
for n,s in edits.items():
    (app/n).write_text(s)
dst.write_bytes((local / dst.name).read_bytes())
