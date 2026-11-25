# SuperJ6 Delay: real original Tempo Sync Enable (Build9 ID118)

Same typed host-only Delay24–27 base, with this delta applied **after**
sj6-delay-typed/delay-typed.patch on that packet's exact prerequisite.
Replay via Bash replay.sh (syntax-checked; individual equivalent exact new
and previous reader builds run). Five patched source files pinned.
No package, native per-device map, hardware, Git, publish, new saved
object or release change.

**Authenticity.** Build9 ARM64 AU SHA-256
a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1.
FXPanel NIB 89bd6d187d9c424f25bf719d817716ef8305d1a2b1c518159ccd0990b53cf913
has unhidden ToggleSwitchSaga outlet delayHzToggle. Its Objective-C ivar
offset storage 0x1007be4d0 is loaded at 0x1002a03c0–0x1002a03cc; the outlet is
bound from the instance's Conductor tempoSyncEnableControl offset storage
0x100809d88 (0x1002a03dc–0x1002a040c). Its original control carries numeric
ID118 (0x10025d9a4–0x10025d9b8). The default helper starts with +0 and
dispatches ID118 via table entry +3 to the unmodified return 0x10001f2e0;
the instrument constructor independently clears byte +392
(0x10036e4e8, offset 56+232+104). The UI callback at 0x10026ee7c invokes
0x1000f11f0: compare raw >0, set the active owner byte +392, perform the
current Time sync application (0x1000ecd50; reload installed raw time and
call ID25 at 0x1000ece1c). The ID25 time path reads that same mode at
0x1000f0914 and selects the original 18-division table and owning transport
tempo; prior 858-check packet independently covers all 128 detents.
The local DSP switch table's number 27 is a different internal namespace
and is not this original visible control's ID.

**Typed runtime.** Existing DELAY page retains Enable/Time/Feedback/Mix;
separate DELAY CLOCK → K1 Clock Free/Sync only under the admitted typed
developer scope, with the other three knobs empty. Always same Time raw,
immediately reevaluated with current sequencer clock when switching.
Each owner uses its existing b16 bit0 enable and b16 bit1 sync in the fresh
typed row, rest b17–19 unchanged. This preserves previous free interpretation.
The exact predecessor is expected to refuse populated sync-bit media; a
mode-cleared otherwise identical fixture remains accepted. No format of
this proof is a release-compatible or approved migration.

**Bounds.** Tests explicitly compile the 250 ms profile at 44.1 kHz
(11,026 cells/channel including guard; 176,416 B stereo float for both
owners). A new impossible Time/Clock pair is refused before persisted
changes. If a dipped live/reloaded BPM cannot represent the authored setting,
the engine keeps its saved raw mode/value, reports TOO LONG on TIME, and
outputs exact dry while the ring advances; feasible BPM resumes. This is a
local safety policy, not the original plugin's long-ring clamp, audible
timing, latency, dynamic tap/AU A/B or product resource admission.
Full free 1.5 s float stereo for both owners would still cost 1,058,416 B;
longer sync taps can exceed even that. No MIX bus borrowing.

The receipt covers true both-owner physical selectors, all four existing
Delay controls, measured DSP targets and reload with changed tempo, exact dry
and history on refusal, malformed packed mode, unchanged non-owned bytes,
Undo/Redo, staged/native virtual reopen and recovered read-only behavior.
Edit/tempo/reopen do not persist anything without explicit fixture Save.
