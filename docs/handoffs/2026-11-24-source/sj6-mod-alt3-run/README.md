# ALT3 free-running receiver: real-input host gate

Source-mod-wheels plus pinned SJ6 ALT1 vibrato-destination patch.
Original parameter, table and oscillator evidence in ../sj6-mod-alt3.
No shipping menu, format or other receiver. Only X0X_HOST plus
X0X_SJ6_ALT3_WITNESS compile the receiver. Explicit running free-clock
qualification and verified ramp length required before admitting destination3.

Original ramper 0x1000049a0 → 0x100004c54 uses Float32 FMA from its last
published level at modulo-8 sample cadence. DSP construction initializes
both default durations to 10,000 frames. Running Swift setters reach
AUParameter setValue: via 0x1005e68c0; observed DSP default is not proof
every external AU event uses this duration. The host effect preserves
offset and phase over sliced spans, uses the authenticated positive sinusoid
as a 4096-point table interpolated by one shared 24-bit oscillator, and
processes after instrument HPF and before downstream compact chorus.
No absent original graph stage is simulated.

Authentic tempo grid (0x1000f00e0-120; jump table 0x100609860):
x=.9f*(CC1/127f), index=round-half-away((1-x)*17), frequency=(Saga BPM/240)
times indexed [64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125].
The rounded reciprocals are original literals. Tempo branch intentionally
refused: this fence does not own original tempo-sync flag transitions,
Saga clock events or a trusted independent running graph state.

Both owners exercised exact extracted SMK ADC scanning and real queued
CC1 on a held note, audible active ramp, no peer or saved Sound mutation,
zero unity and inert default. Separate interpolation assertions compare
5+7+12 vs 24 contiguous frames and exact mod-8 receiver observation.
No actual AU scheduling/re-arm, original complete audio equivalence, hardware
or native resource qualification follows. No Git/package/device/publishing.

Reproduce in disposable source-mod-wheels copy with usual generated inputs:
apply prior sj6-mod-wheel-vibrato/vibrato-destination.patch, then this
alt3-fenced.patch. Copy alt3-app.c as tests/host/sj6-mod-wheel-host.c.
Extract wheels.inc with:
  sed -n '/^static int smk_pitch_parameter(int v)$/,/^}$/p; /^static void smk_wheels(void)$/,/^}$/p' firmware/src/app/main_fm1.c > tests/host/wheels.inc
Run the packet's run.sh from disposable source.
