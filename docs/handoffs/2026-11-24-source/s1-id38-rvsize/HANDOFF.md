# Synth One Reverb Size: original Feedback ID38

Proven at AudioKit/AudioKitSynthOne 6466a3715c96b7ecf1dd255a218cbd571408a314:
Effects/Base.lproj/Effects.storyboard explicitly identifies Reverb Size,
EffectsPanelController.swift binds reverbSizeKnob to .reverbFeedback,
HeaderViewContoller.swift shows Reverb Size as percentage, DSP/Kernel/
S1DSPKernel.hpp declares 0/.5/1, and S1DSPKernel+process.mm feeds it to
Costello feedback per sample before the independently gated wet crossfade.
Original revsc applies feedback before its filtered tap output; zero is
not a missing early-reflections pathway in this particular original kernel.

The typed host proof plugs the same normalized value into existing compact
receiver ID38. It does not re-voice compact room or try to replace Costello:
existing four fitted feedback gains use per-line length powers and an
existing decimated/hot receiver with ordinary gated crossfade. The independent
ID51 Reverb Mix LFO coefficient and Reverb switch are untouched. UI is
existing FX SWITCHES K4, RV SIZE and percentage; K1/K2/K3 stay ID41/37/51.
For legacy or non-S1 Init K4 isn't added. No new route, effects topology,
friendlier numeric interpretation or preset changes.

Only typed sidecar position b12 is reserved. 129 binary-exact detents 0..1
include exact 0.5, encoded by canonical zero. b6 A4 remains independent;
b7–11 remain unowned in this donor and future overlays must commute rather
than reinterpret them. Real edit path checks typed coherent active owner,
reads/writes only accepted owner and refuses recovered previous sound edit.
Undo uses actual project history/audio hold; save/reopen uses the qualified
native-layout bound tuple and portable CRC. It is current owner reverb
feedback, not a new frozen note lock payload.

Test-only observation taps record both compact FDNs at their existing
cadence and their actually applied wet multipliers. Feedback changes only
selected owner's target, changes actual gated-on compact render at endpoints,
and fully settled gated-off multiplier is below 1e-8 (not an instantaneous
hard-zero claim; the existing front-end smoother still runs). This is not
original Costello sample parity or native timing/format qualification.

Replay: sh replay.sh ../controls-init-only/source-mod-wheels
Exact all-four + A4 sealed donor, one temporary source, no default activation,
physical device, assets, Git or release writes.
