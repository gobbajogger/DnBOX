# SuperJ6 behavior brief for an independent DnBOX implementation

## Purpose and permitted inputs

Implement SuperJ6 behavior using existing efficient DnBOX building blocks and independently written additions. This folder contains control identities, numeric targets and observations from already recorded comparison output. It contains no original engine code, disassembly, source addresses, proprietary coefficient tables, or factory preset payloads. The original instrument and component recordings are comparison references only. Do not reuse source-aware prototype implementations, including the previous direct Dunne envelope adaptation.

Start from the authorized current DnBOX source snapshot supplied by the owner. The 352-byte host compact-Juno state is a useful existing framework. Its sound and 18-control API do not implement the SuperJ6 contract. Six stock note slots are an initial prototype capacity, not proof of original polyphony or final product acceptance.

## Control and storage contract

The accompanying `public_control_catalog.json` distinguishes 218 app control identities, 170 positions in the main preset ordering, 167 applied controls after three global-state exclusions, 174 distinct serialized names observed in the supplied 640 programs, and two unresolved legacy names. A separate recorded component has 52 selected parameter IDs. A requirement described as “64 controls” must be reconciled with those dictionaries; none of these counts is interchangeable.

Store raw input values separately from prepared DSP parameters. Do not put every field through the compact-Juno `0..1` setter or an eight-bit macro buffer. Preserve absent versus present controls, including a present zero. In all 640 supplied programs the two layer-sound selectors are present with zero; substituting an absent-field default changes the meaning. Preserve unknown legacy fields for future interpretation and report their unresolved sounding behavior.

The public catalog identifies global key-lock IDs 212–214 as preserved during an engine preset application. ID 207 is reserved and is not a musical knob. Existing recognized legacy fields 18–21 and 146 sit outside the normal main-preset order. The unresolved names are `neonlimiterAmount` and `neontubeRageAmount`; neither may be silently equated with a current compressor or drive control.

The following are real input-domain counterexamples, not proposed new ranges:

| Input | Required treatment |
| --- | --- |
| Master volume value 1.32 | Retain raw value; determine the qualified physical gain instead of clipping to one. |
| Pulse/saw level 1.001 | Preserve the input; a small excess may be part of saved endpoint encoding. |
| LFO destination selector 70 | Retain selector identity; it is not normalized gain. The route-selector-to-app-control dictionary remains unqualified. Do not assume it means app ID 70. |
| Pad destination 9 | Preserve enumeration and qualify its separate routing dictionary. |
| Sequence division −0.008403362 | Preserve transport value until boundary/migration behavior is established. |

For the first independently implemented adapter, raw binary32 values are a defensible correctness baseline: 218 values occupy 872 bytes before flags, routes, synchronization and other state. This is arithmetic, not a complete target-memory allocation or permission to exceed the owner’s budget. Optimize only after behavior-preservation tests exist.

## Numeric knob targets

`numeric_behavior_targets.json` gives synthetic scalar examples and limitations. These targets establish control meaning; they do not define the oscillator, filter or envelope algorithm.

| Raw knob | Amp attack seconds | PWM duty when enabled | LFO frequency in Hz mode |
| ---: | ---: | ---: | ---: |
| 0 | 0.002 | 0.5000 | 0 |
| 0.25 | 0.160199797 | 0.3825 | 0.158219574 |
| 0.50 | 1.592874083 | 0.2650 | 1.591072968 |
| 0.75 | 6.139898460 | 0.1475 | 6.138665793 |
| 1 | 16 | 0.0300 | 16 |

Attack app ID 36 follows the published numeric target `0.002 + (16 − 0.002) × u^3.33`. This does not establish decay/release knob laws or their curve shapes. PWM narrows as the knob increases; app IDs 78/79 target a 50%-to-3% duty range, and IDs 210/211 independently enable PWM. Layer waveform stops at 0, 1/3, 2/3 and 1 identify Sine, Triangle, Saw and Square. Oscillator-range stops 0, 0.5 and 1 identify 16-foot, 8-foot and 4-foot. LFO Hz mode uses the numeric target `16 × u^3.33`; tempo mode requires its own division table and boundary behavior.

The stock normalized Juno setter cannot span these targets. Its attack stops at 3.002 seconds, width at 0.1–0.9 before the render clamp of 0.05–0.95, and PWM LFO at 0.1–8.1 Hz. Inverse mapping can align some scalar values, but cannot repair the waveform, envelope, modulation or routing differences. Use physical prepared parameters for the new implementation rather than forcing them through the old macros.

## Amplitude and filter envelope behavior

App amplitude controls are IDs 36–39 and 119–122. Filter-envelope controls are 132–135; pitch-envelope controls are 138–141. Link flags are controls with explicit behavior, not permission to share a mutable envelope state unconditionally. A part must support independently evolving amplitude and filter contours; simultaneous notes and layers must not overwrite each other’s state.

The actual recorded component’s amplitude fixture takes six physical values in the order **attack, hold, decay, sustain, release hold, release**. Time values are seconds; sustain is a fraction. The filter and pitch fixture interfaces each take four ADSR values. A generic four-argument ADSR comparator is therefore an incomplete amplitude contract.

`recorded_envelope_behavior.json` contains the exact input arrays, note events, frame counts and output hashes of six retained original recording cases. `amp_observed_levels.csv` contains 2,560 rows of measured amplitude behavior inferred by dividing matched recorded signals with a common source. The data includes each 16-frame interval’s first and last observed level and the residual against the line between them. This is a measurement of the recordings, not original implementation code or a prescription for internal state structure.

The baseline at 48 kHz uses amplitude `[0.01, 0, 0.05, 0.7, 0, 0.1]`, note 60 at frame 0 and note-off at frame 4096. Selected inferred amplitude values are approximately 0.06743794 at frame 3, 0.19491958 at frame 48, 0.42627948 at frame 120, 0.70251555 at frame 240, 1 at frame 479, 0.90858929 at frame 720, 0.79439916 at frame 1200, 0.70832930 at frame 2400, and 0.7 at frame 3000. The initial interval is flat; later intervals change smoothly. A staircase envelope is not supported by the observation.

For the same source, note, timing and physical envelope, velocity 127 versus velocity 1 scales the recorded amplitude by 127 within float rounding: the maximum absolute residual is 6.6589564e−8. This sampled component path therefore uses a linear velocity gain. Do not substitute Synth One’s squared-velocity response on that basis.

Other retained cases cover nonzero attack/hold/release-hold, zero-duration stages, irregular-block retrigger, and sustained playback. The longer hold case uses `[0.03, 0.02, 0.2, 0.4, 0.04, 0.3]`, note 60/velocity 100, note-on at frame 0, note-off at 16384, total 32768 frames. The zero-duration case uses `[0, 0, 0, 0.7, 0, 0]`; the last 4096 frames after note-off are zero. Do not infer all early-release or voice-steal behavior from release-after-sustain cases.

**Filter-envelope gap:** the music captures configure filter ADSR `[0.02, 0.1, 0.7, 0.2]` and pitch ADSR `[0.03, 0.2, 0.5, 0.3]`, but those modulation paths are bypassed in the music baseline. They establish physical input identity, not an audible filter-envelope curve. Other selected-component control recordings exercise enabled filtering; they still do not establish the complete app knob-to-seconds mapping or all filter alternatives. Keep that qualification open. No generic-Dunne or Synth-One-shaped envelope result may be promoted to complete SuperJ6 acceptance.

## Reuse plan and expected differences

| Existing DnBOX element | Useful reuse | Behavior still required |
| --- | --- | --- |
| Compact-Juno voice ownership, note events and allocator framework | Bounded caller-owned storage and event entry points | Original legato, duplicate-note, restart, steal, damper and voice-retirement behavior; second independently controlled layer. |
| Compact-Juno phase accumulation, polyBLEP saw/pulse, fast math | Efficient independent oscillator primitives | Four waveform identities, pulse direction/range, both layers, detune/range/pan, noise, and separately qualified sub-source character. A raw half-rate square is not automatically the original sub. |
| Compact-Juno envelope state conventions | Independent state layout and observation plumbing | Recorded amplitude shape, holds, release holds, separate filter/pitch contours, zero-duration boundaries and live edits. |
| Existing filter blocks and coefficient preparation | Efficient starting components; cache parameters when invariant | Original filter alternatives, cutoff/resonance transfer, key tracking, velocity and independent envelope modulation. Stock two-pole Juno output is not a proven counterpart. |
| Existing short-delay/chorus/ensemble primitives | Ring-buffer access, interpolation and modulation code | Original chorus I/II and version controls, wet law, polarity/stereo behavior, feedback and depth/rate domains. |
| Existing Chicago effects | Independent compact implementation candidates | A selected-algorithm union cannot represent simultaneous chorus, delay and reverb tails. Reusing its setter can import unrelated wet floors, fixed delay or halved tempo divisions. |
| Existing timing and automation infrastructure | Scheduling, parameter delivery and ownership | Three independent LFOs, mode/retrigger/delay/snapback, target dictionaries, original arp/sequence events, pitch/tremolo and pad behavior. |

No row in this table claims an exact original sound match. Code and immutable lookup tables may be shared. Concurrent voices, layers, two parts, and effect-tail histories need independent live state. Preserve existing global send effects; an already owned global delay is not free private-engine memory.

## Smallest defensible architecture

Use one per-part physical-control adapter, a bounded note-slot array with separate layer/amp/filter/pitch state, a small per-part modulation/event state, and a per-part serial/parallel effect graph whose topology is qualified against recordings. Share code, immutable math tables and serialized workspace where lifetimes genuinely do not overlap. Do not fold envelopes together, collapse independent layers, merge different private FX tails, or shorten tempo delays merely to make a state union fit.

Start dry: one layer, physical envelopes, waveform/PWM/range/gain, then the second layer and sub/noise routing. Require reference comparisons at each added behavior. Extend filter and modulation only with enabled-path fixtures; add private FX with measured tail behavior and explicit resource ownership. This order makes a mismatch diagnosable and does not redefine the final scope.

## Completion criteria and open evidence

The structural inventory and compiled stock-setter probe pass. The numeric amplitude observations are internally consistent to at most 1.796592e−7 deviation from the observed interval-endpoint line. Those are data/compatibility checks, not a new SuperJ6 render, CPU result, final fit or device qualification.

The remaining evidence includes route-selector dictionaries, app-to-component control bindings, all normalized envelope laws, complete original layer/sub selection, filter alternatives and active envelope behavior, full FX routing and tails, original preset application/migration/unknown keys, and complete polyphony/lifecycle behavior. N01’s historical “14/56 SuperJ6 comparison” has not been authenticated as a SuperJ6 package; existing 13-control/10-music selected-component recordings are usable bounded references and do not close the full-preset gate. Full program/control coverage and the owner’s audio, memory and timing gates remain separate required outcomes.
