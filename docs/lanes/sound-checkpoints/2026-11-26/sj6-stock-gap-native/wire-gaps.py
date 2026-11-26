from pathlib import Path
import argparse
ps=argparse.ArgumentParser()
ps.add_argument('h',type=Path);ps.add_argument('c',type=Path);ps.add_argument('--retain-gaps',action='store_true')
a=ps.parse_args()
s=a.h.read_text()
old=""" juno6_voice_t voice[JUNO6_VOICES];
 uint32_t age;
 float sample_rate,attack,decay,sustain,release,cutoff,resonance,pulse_width,sub_level;
 uint8_t waveform, damper;"""
new="""#if DNBX_SJ6_RECLAIM_STOCK_LAYOUT && !(X0X_SUPERJ6_FACTORY && X0X_SUPERJ6_BUILTIN_INIT)
#error stock Juno layout may be reclaimed only when factory INIT owns all voices
#endif
#if DNBX_SJ6_RECLAIM_STOCK_LAYOUT
 float sample_rate;
#else
 juno6_voice_t voice[JUNO6_VOICES];
 uint32_t age;
 float sample_rate,attack,decay,sustain,release,cutoff,resonance,pulse_width,sub_level;
 uint8_t waveform, damper;
#endif"""
if a.retain_gaps:
 new=new.replace(" float sample_rate;\n#else"," float typed_filter_gap_a[55];\n float sample_rate;\n float typed_filter_gap_b[8];\n uint8_t retired_stock_flags[2];\n#else")
assert s.count(old)==1;s=s.replace(old,new)
old=" float macro[18], bend, hpf, hp_state, hp_input, lfo_phase;"
new="""#if DNBX_SJ6_RECLAIM_STOCK_LAYOUT
 float macro[18], bend, hpf;
#else
 float macro[18], bend, hpf, hp_state, hp_input, lfo_phase;
#endif"""
assert s.count(old)==1;s=s.replace(old,new)
if a.retain_gaps:
 s=s.replace(" float macro[18], bend, hpf;\n#else"," float macro[18], bend, hpf;\n float typed_filter_gap_c[3];\n#else")
a.h.write_text(s)
s=a.c.read_text()
def guard(a,b):
 global s
 assert s.count(a)==1 and s.count(b)==1
 s=s.replace(a,'#if !DNBX_SJ6_RECLAIM_STOCK_LAYOUT\n'+a)
 s=s.replace(b,'#endif\n'+b)
guard('static float midi_hz(unsigned n)', 'void juno6_chorus_init(')
guard('JUNO6_LEGACY(int, active)', '#if X0X_SUPERJ6_FACTORY\n#include "juno6_factory.inc"')
a.c.write_text(s)
