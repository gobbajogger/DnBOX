# Current original Mac ARM64 Main/Sub handoff

Run with authorized newest Mach-O in its private location pointed to by
/tmp/dnbox-newest-sj6-oracle-location, Unicorn 2.1.4 and the existing original
bench on PYTHONPATH from workspace root. Run python sj6-modern-owner/probe.py.
No plugin binary copy, AU registration, Mac runtime, or audio files are used.

New actual compiled AKSamplerDSP constructors, public receiver slots,
actual MIDI leaf/play/stop, render, and internal VoicePrep/cursor hooks
are executed. Unicorn CPU MAX supports the real ARM64 LSE atomic. All
396 VoicePrep rows, six AMP crossings, and six state initializations match
the independent older binary's execution exactly. The hard compares include
velocity, requested note/pitch, held glide field, saw phase and increment,
AMP current, sample cursor, active-vs-pending zone and stored ratio.

Three cue arms start with 36@86 held, or released 11025/22050 frames before
the 48/52/55 chord. Same commit frame 33491 under these input controls,
same continuously advancing old phase/cursor and matching reset order,
no Modern-vs-old intrinsic source handoff skew. Exact old projected
per-receiver setting sequence and deterministic marker sample zones,
not Swift layer/timer, an original sampled waveform or modern capture.
Do not claim identical older/current whole engines or insert a fixed delay.
