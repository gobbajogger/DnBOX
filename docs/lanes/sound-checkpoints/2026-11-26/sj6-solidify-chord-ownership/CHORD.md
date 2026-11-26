Solidify chord ownership correction not justified. Same exact saved public cue throughout.
Authentic controlled current ARM Main receiver proves slider-gated co-timed owner:
with DSP param35=0 target immediately uses the final accepted key with phase zero
and a single Attack stage, no pending commit on short release. Param35 maps to
DSP byte +0x154; param26 mildly changes enabled glide, #39 inert. Param35=1
diagnostic demonstrates distinct direction/order-sensitive pitch clock, ~131Hz
instead of ~196Hz at first 16 frames, still <138Hz at 768 frames, dependent on
first key. DSP receiver ID is not itself proof of installed AU routing.
Genuine installed dry and both authored repeat captures identify the final
~196/~98-Hz ridges already in the first 2048 frames and independent later
windows. This excludes the slow controlled transition as an implementation fix;
does not fit shift, reroll Vintage, or prove unmeasured shorter startup motion.
Actual typed phase trace uses fresh attack and phase zero for MIDI60→64→67
and retains selected key67 sustain on offs 60 and 64. Off67 enters its release
without resetting phase. In controlled source receiver, offs 48,52 similarly
leave key55 untouched; off55 starts release without phase reset. Controlled
long-release variant is a different topology and not presumed from old precedent.
The standalone controlled receiver uses mapped keys (−12 Range), known fixed
velocity, A440 and source binary exact signature; project ADSR and B layer
physical semitone control are not claimed faithfully calibrated in these
receiver sweeps. Typed taps do not change output bytes.
Reproduce source runs from cwd with PYTHONPATH including original authorized
Unicorn and emulator, save as chord-native.json, chord-order.json, chord-param.json,
then run chord_score.py on same dir. Typed uses previous suffix build exact
flags with chord_typed.py caller overlay in that temporary folder.
