# Synth One independent Delay and Reverb gates, Init-only

Base: exact combined `controls-init-only/source-s1-mono-sj6-id1`,
manifest prefix 602ed70c. Patches separate; do not apply the UI patch unless
layout capacity is separately approved and verified. No native, device,
package, Git or warehouse writes.

**Original evidence.** Pinned AudioKit `DSP/S1Parameter.h` and
`DSP/Kernel/S1DSPKernel.hpp:393–400`: ID37 Reverb On default ON and ID41
Delay On default OFF, domain 0..1. Pinned `S1DSPKernel+process.mm:229–293`
continuously processes ordinary wet history and multiplies selected wet
mix by its enable; reverb also multiplies by its selected LFO. Existing
DnBOX Effect Mix controls set ID44/40 only; they do not actually auto-enable.
This delivers fixed-resolution panel OFF/ON, not a claim to original
continuous toggle smoothing, reverb tone, delay tails or complete AU parity.

**Backend.** Extend the existing typed byte30 FM/AMP route radix 25 with
independent ternary OFF/ON/absent Delay and Reverb digits. Codes 0–24
are unchanged, 25–224 are previously invalid in exact prior and cannot
enter historical slot-2 migration. Reject >=225 and missing marker.
Absent digits reapply exact program word. Record sizes / sidecar claims do
not expand. Existing DSP multiplies the target gate into the mix as before;
explicit Delay mode reserves real delay history once in the existing
bounded pool (stock delay-off dummy partition remains byte-identical
until an explicit claim). Repeated toggles do not clear or resize history.
Legacy authored Delay already fully enabled is not repartitioned. Reverb
never changes its history. Panel values and underlying Mix remain separate.

**UI.** Companion proposed FX SWITCHES after EFFECT, K1 Delay, K2 Reverb,
with unrelated UI unchanged, ARP still inserted before EFFECT. Installer's
typed capacity prototype can display the row in host but its native fit is
currently red; when composing after eventual tight typed-capacity patch,
use PT_FX_BYPASS title ID registered as FX SWITCHES. The delivered UI patch
changes no MAXPAGES and compile-time rejects an insufficient cap. The
physical host test used a strictly isolated 16-row test copy (not a
product patch, group budget, or native footprint proof).

**Evidence.** Exact host PCM both gates absent unchanged (4,096 stereo
frames); left/right owner-isolated audio differences for each switch and
bit-exact peer, selected mix unchanged bypass/re-enable, real delay pool
bounds and ring pointer continuity. Both-owner cold project reload,
actual sequenced lock/release, earlier Mono, Glide, coarse, detune, link
unchanged, and exhaustive existing route/high-byte marker checks. Old
exact parser refuses both real projects and subsequent save, no writes,
104 assertions. Appended owner readback exists only in scratch. Delay's
active history costs stock bounded DSP work; native fit/timing not measured.

Replay: stage a private copy of exact preimage; apply backend. For
physical tests only apply UI patch in the same scratch and raise bare
capacity 15→16 solely as a test scaffold (or use qualified capacity).
Append ID30 handoff's test-only host_coarse_owner accessor to engine.c.
Use `sh run.sh [W [SOURCE [OUT]]]` with private checkout path. Use
`TEST_FLAGS=-DS1_FX_BACKEND=1` without UI patch for backend on unchanged
page capacity; load the fixture's actual exported files in prior
firmware with `TEST_FLAGS=-DS1_FX_OLD=1`. `S1_FX_PCM=FILE` exports
untouched Init from either side, `S1_FX_RED` proves the old endpoint
absence when running without PCM env. No owner renders or banks in packet.

## Exact typed UI composition, superseding the fenced UI patch for integration

Use `s1-fx-bypass-ui-typed.patch` **instead of**
`s1-fx-bypass-ui-after-capacity.patch` on exact parent
`controls-init-only/source-s1-arp-fix`, `UI-CAP-POST.sha256`
`7ff48492e39aad46c1a0585bd4f361c287822e03fcbb120f0bb4f05f5df0fcc1`.
Apply the same six-file backend (zero-fuzz), then the typed one-file UI
patch. Existing title IDs stay unchanged; PT_FX_BYPASS added with text
FX SWITCHES. No layout edits/capacity bump. Actual typed test with
`TEST_FLAGS=-DS1_FX_TYPED=1` verifies per-owner physical order ARP,
EFFECT, FX SWITCHES, OUTPUT, SOUND, alongside the real gate/PCM/peer/ring/
old write boundaries. Untouched 4,096 Init stereo frames remain exact
against earlier narrow preimage. No native image or release qualification.
