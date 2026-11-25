# SuperJ6 ALT3 synchronized and free clocks on the same audible host path

Apply after exact source-mod-wheels, prior ALT1, and sibling
sj6-mod-alt3-run/alt3-fenced.patch. Patch is two DSP files only inside
existing X0X_HOST + X0X_SJ6_ALT3_WITNESS guard. No UI selector, new
user mode, save format, preset, clock writer or other control.

Exact original Build9 AU SHA a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1.
Mapped ObjC KoFMInstrument class 0x1007b2448 ivar table 0x10073c8f0
declares isTempoSyncd at running offset +392; seq at +176 (0x10073c390).
Not a per-LFO flag. Explicit trusted host flag models that existing
owner mode, no general LFO substitution. With mode ON, Float32
x=.9*(CC1/127), index round-half-away((1-x)*17), exact Build9
[64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125],
Hz lives in Float32 after the original Double (BPM/60)×grid×.25.
Tempo read is from actual compact transport prepare, not a free LFO.
When a qualified synchronized clock changes, the rate starts its
own running ramp while the separately observed depth keeps its
current epoch. Zero or missing synchronized clock denies new wheel
edits without mutating either endpoint; no automatic tempo estimation.

Explicit arming accepts only the 10,000-frame DSP constructor duration
profile and a known mode. Zero, 9,999 or unspecified profile refuses
before any writes. Re-arming active receiver refuses. Original Init
graph setter 0x10005accc..0x10005ad00 seeds rate 3 Hz/depth zero; only the
armed test receiver adopts it, unarmed default is exact no-op. Applying
a live AU event with unknown duration is still not authorized by this
constructor proof; do not wire the host arm to arbitrary callbacks.

1,130 assertions across both owner real SMK ADC/CC1, active audio
post-HPF/pre-chorus, phase/interpolation and zero unity, free and
synced independent ramps, sequencer 143 BPM live transport edge,
all 128 sync rate selections at 93.5 BPM, strict unknown-profile/clock
rejection, no peer or project writes. Exact original live AU scheduling,
other graph events, native performance/layout and activation remain
unverified, no published or physical changes.

Replay as sibling sj6-mod-alt3-run, applying this patch last and using
alt3-sync-app.c in place of its fixture.
