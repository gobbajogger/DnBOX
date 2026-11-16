# SynthOne native pre-FX source closure

Pinned derivation: 56079bbdcb753c37d65aedd17ce49ba71c261797.
`SOURCE-MANIFEST.json` identifies every copied dependency by bytes and SHA256.
MIT notices are retained in `experiment/notices/`. SDK C++ headers and compiler
binaries are not included here. This closure is NOT an active application engine.

The initial source closure was copied byte-for-byte from the authenticated native
full-sequence build, then all29 runtime units were freshly compiled again from
these repository paths with the pinned pi32v2 compiler. Native object evidence and
scope are in `docs/sound/v2-native-component-result.json`; no firmware link, CPU,
full-FX, factory-bank or all-engine fit is claimed.

Native compile policy: original C++11 overload headers, `S1_NO_DOUBLE`,
`-target pi32v2 -mcpu=r3 -mfprev1`, no fast-math, no exceptions or RTTI.
Integer binary64 clock helpers preserve original clock semantics. Do not substitute
a generic host build command or silently use integer abs overloads.

Lifecycle: caller-owned stable-address `s1scheduler::SourceScheduler`, serialized
control/render calls, explicit original-source table metadata and compact wave pack,
150 physical parameters and128-entry tuning supplied on init. Public note handling
uses start/stop, not the sequencer's lower-level on/off sinks. No hidden allocation
is authorized. The eventual C ABI and ownership reuse must be reviewed before
integration; current10532-byte target owner cannot simply be appended to the
existing pool. Parameter frontend/factory routing and post-note FX remain open.

Derived changes preserve the prior input SHA in each changed SOURCE-MANIFEST entry.
The serialized held-note AE snapshot now aliases its owner; native owner8996 bytes
(previous10532). Both full original event/state and audio comparison tests pass;
no concurrent UI/audio use is permitted. This is still not combined fit.
