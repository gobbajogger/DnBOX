# Fenced original Synth One clock receiver: BPM 64 / factor 147

Host-only handoff. No modifications to product, presets, storage,
ARP activation/modes, existing transport (including STOP), or release gates.
Exact compared product context is source-s1-reverb-lfo manifest
f74afed6ce7fc9087c4b1e3a8b2e53a3e5e5e2ad57fc1ea878ce5c49399b077b.
Clock code has no dependency on that tree and exposes no engine/UI route.

Run `sh perf-final-proof/s1-arp-clock-receiver/replay.sh WORKSPACE_ROOT`.
Source input manifest pins the pinned AudioKit 6466a3715c96b7ecf1dd255a218cbd571408a314
mechanical original, its serialized note adapter, and the independently
recovered 19-stop factor table. Only derived adapter and executable go to /tmp.
The adapter swaps out reset/threshold/index calculation, not note generation.
The oracle is the mechanical original, not the modified adapter.

Each serialized owner admits finite Float32 1 through 200 inclusive and the
distinct 0-based original stop list; invalid edits don't change phase/params.
The rendering caller supplies the fixed positive finite sample rate. No
fast-math. The original rounds BPM division by 60 in Float32, then divides by
the owner sample rate in Binary64. Caching this increment at a BPM write
changes no floating operations on phase under fixed rounding/sample rate.
The receiver deliberately keeps Binary64 per-sample addition and both original
fmod operations, first-held increment/trigger, zero↔held resets and pending
note behavior. It doesn't extrapolate future events or apply a global nudge.
Beyond the original signed int beat index domain it refuses another trigger;
no practical overflow scenario was exercised. No concurrency guarantee.

Both actual source scheduler note pipelines were exercised with 38 stop/rate
pairs, each with separately configured interleaved owners, distinct roots and
velocities; compare full frame- and order-stamped beat/on/off callbacks,
including initial original zero-note flush, holds, rests, edits while held,
release/re-entry, and disable/re-enable. Observed 123,716 identical callbacks.
Slowest true case 1 BPM/factor8 really arrives (second note sample 21,167,998);
this includes the formerly unrepresentable uint32 Q8 period.

Remaining dependencies: actual product ownership/activation and independently
safe persistence for both controls, source-authentic arbitration with DnBOX
transport, native flash/time and platform arithmetic proof. The host receiver
adds no storage or production default behavior. Do not treat this as a
native-fit or real-time performance implementation.
