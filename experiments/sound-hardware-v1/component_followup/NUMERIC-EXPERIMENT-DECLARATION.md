# Source-free S1 envelope experiment

Declared before execution. No original implementation, reference source,
earlier adapted source, or waveform asset is an implementation input.

The 3 static and 7 live output recordings support a compact hypothesis:
per-sample exponential approach; an attack time constant of 0.6 times the
physical attack setting and a transition at level 0.99; decay and release
time constants from their physical settings; the current sustain target
changes immediately while the current decay coefficient remains latched;
time coefficients latch when entering their stage; a gate restart retains
the current level. Those are hypotheses inferred from recorded values, not
claims about the original implementation.

The comparison requires **zero different binary32 words**, finite output,
unaltered event frames, gain 1 and zero alignment shift. It spans all 379,288
supplied scalar samples. There is no threshold search. Four deliberate wrong
behaviors must reject: nominal-duration attack coefficient, sustain snap,
reset on retrigger and immediate refresh of a running time coefficient.

The output is a component proposal. Passing would not qualify any complete
preset, production path, target coefficient approximation, filter, FX,
allocator, memory fit or native timing. Failing is retained unchanged.

Only Python scalar arithmetic and existing numeric packets are involved;
there is no native compilation or original executable. The root owner must
release its serialized link window before this check runs. Authoring this
packet does not reserve that window or start a test.
