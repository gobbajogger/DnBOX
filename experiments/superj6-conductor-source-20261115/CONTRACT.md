# Caller, numerical and effect contract

## Common ownership and numerical boundary

Calls are cold and serialized. Callers provide valid live input/state/string spans, writable aligned outputs, and disjoint provider storage. Interval/overflow checks cannot establish arbitrary pointer validity. State APIs permit only the exact in-place cases stated in their headers, never arbitrary overlap. Callbacks must not throw. Production code does not allocate a hidden backend or imply that a missing capability succeeded.

Binary32 and binary64 staging is intentional. No floatification, fused-multiply-add substitution or global fast-math assumption is allowed. Platform sine, exponent/power and entropy results remain external dependencies; host results do not establish original iOS math/FENV/NaN identity. The Python reference validates source-width/type admission, including exact uint64 generation/token identities and finite binary32 cutoff words. Finite binary32 subtraction overflow in ACK comparison is a valid non-close result, not a Python conversion exception.

Command planners preserve outputs/state on their documented preflight failures; pure providers may already have been called. Streaming executors have external effects and report attempted/completed prefixes. A failure after an effect is not rollback and does not authorize replay. The original command-planner result must not be installed early as if its commands had already executed.

## Prefix, time and ordered live reads

The prefix advances phases in original order 1,3,2 using old delay-finished flags, captures the distinct fourth-rate factor, constructs tables in order 0/2/3/1/4/12, then updates held samples. All 256 sine calls and ID12 entropy/discarded/rejection draws are retained. A single 256-Float scratch table is reused only after selected words are copied; it is not a cache or skipped constructor. Failed prefix preparation does not publish pending output/state, but consumed entropy is not rolled back.

The time stage advances delay1 and delay3 with their actual comparisons. It does not invent symmetric LFO2 delay gating. The source-point collection-plus-noise global-vibrato restart is a distinct callback, not all-notes-off or a general engine reset.

LFO1 applies its first destination and its amount threshold. LFO2/3 each have two destinations: the delta is captured once, but the second destination/base is reread after earlier callbacks. Cycle flags are published at their actual stage point. Mode changes alone do not silently clear completed cycles. LFO1's stored second destination/amount is not invented as an extra effect in this inspected path.

The separate fourth LFO retains the captured binary32 rate factor but reads live targets for its effects. Its terminal branch applies frequency twice in source order, including the intermediate reread. Frequency, depth and global-vibrato restart have different receiver groups. The tempo-sync flag must reflect the actual source branch, not a guessed preset interpretation.

Ordered cutoff publishes the emitted current value before the Core/notification callbacks, rereads after them, compares the captured target, and only then snaps/clears state after checking actual Core cutoff. Signed zero is preserved at source-width boundaries. Ordered tick composes readiness, prefix, time, three LFOs, cutoff and fourth LFO. Effects may change later live controls/cutoff within the header contract, but may not reenter preset/note/tick or mutate timing/cycle/held-note ownership. Full reentrant original ownership is not implemented.

## Reset, retrigger, clocks and restoration

Full reset and selective retrigger are different operations. Full reset preserves held4/delay state where the source does; selective retrigger is gated by new-note and retrigger1/2/3, not fourth-LFO-only requests. Both invoke the actual ordered zero-dt tick where specified. Zero dt still consumes required entropy/table work and may emit effects. Failure after reset/restart is partial.

Top-level render block duration is distinct from internal voice chunks. Beat catch-up retains repeated binary64 0.0125 subtraction and the original large-gap handling; it is not floor-division scheduling. Registration/transport validity remains caller work. First held-note ownership is distinct from active sampler voice count. Restoration retains first matching control identity, duplicate writes, the cutoff special case, missing-control suppression and global sentinel218. No general destination mapping or nested note-routing implementation is inferred.

## Preset passes and defaults

`plan_main` remains the atomic command planner. `execute_main` is a separate streaming API: exact170 order, skip212/213/214, first-match immutable records, otherwise exact218 defaults. It immediately dispatches each effect at its sequential point. Caller mode true applies volume normally; false requests the guarded0.15-second defer. Delay-enable preparation precedes its control. The adapter must actually choose the original control callback versus parameter-setter behavior and copy any deferred command reference.

Complete potential capabilities and missing derived-math providers are checked before effects. This conservative preflight intentionally requires DelayPrepare even if a particular preset would not enable delay. Later math/backend failures retain explicit attempted effects, completed bindings and failed ID. Records and their control/scheduler owners stay pinned and immutable throughout the call.

All218 defaults remain:189 binary32 literal words and29 intact four-word binary64 inverse-parameter expressions. The split literal/index/expression representation changes storage only. It retains exact constants, formulas, provider calls and ordering; no missing default is replaced by an invented zero.

`execute_pwm_postpass` is explicitly separate and must receive the four routes captured after the main pass, with pinned actual base-control identities. Only captured route78/79 reapplies base controls184/186/188/190 in order. It searches the base-control identifiers, not destination78/79's preset values. Equal destinations do not deduplicate effects. Earlier callbacks cannot change the captured selection. Success says only that this postpass completed. Tuning, library resolution, transaction/migration, presetLoaded publication and full loader success are not implemented here.

Deferred volume execution requires the actual model-lock/owner/generation/apply/clear/unlock capabilities. A stale generation neither writes volume nor clears a newer pending value. Callbacks must implement their stated effects; a fake recorder used by tests is not a production backend.

## Required notification and vibrato-depth backend obligations

The abstract `dispatch_global` and `post_modulation` effects stand for the original synchronous notification helper carrying normalized value and parameter identity. They are NOT a second raw DSP/global setter. For global parameter7 while cutoff smoothing is active, the helper substitutes the actual currentCutoff value. The adapter must preserve that override, synchronous observer ordering and live-owner reads. Notification name/object derivation, all observers and their downstream effects are not supplied by this package; a recorder or a raw ID setter is not a qualified replacement. The separate asynchronous enable notification is not interchangeable with this helper.

The original vibrato-depth backend first computes `12 * pow(double(Floatdepth), 3.03)`, before reading the actual LFO1 rate and comparing it against binary32 .02. Above that threshold, it narrows the already computed mapping to binary32, then widens for the depth receiver calls in order136/144/168/152/160. At or below the threshold, it discards the mapped value and instead sends zero frequency to receivers136/144/152/160. The mapping and its math-provider call must therefore remain unconditional and precede the live-rate read, even when the mapped value is discarded. Thus a depth command does not unconditionally mean a depth write. The frequency, depth and global-vibrato restart receiver groups and effect ordering remain distinct. The required real adapter must implement the actual branch, live rate, numerical staging and receiver identities; the abstract controller callbacks and host command tests do not qualify that backend or establish its original audio behavior.
