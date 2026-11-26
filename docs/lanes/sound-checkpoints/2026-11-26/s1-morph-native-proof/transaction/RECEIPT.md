# Guarded 909 voice future (quarantined, does not link yet)

Apply wire.py to a private sibling directory containing a copy of s1-morph-native-coldread/tree. Do not build firmware for installation using this experiment. run.sh uses the unchanged, model-specific guarded companion linker and genuine PI32v2 tools. The fixture retains both synths with arp, no PSRAM, and INIT-only builds.

Native APP delta vs exact cold-reader baseline: .text grows 448 B on each board. .ram_text/.c1_text/.data unchanged. .bss grows 16 B inclusive of layout alignment; each original_runtime_workspace remains 5120 B. APP overrun is 100 B Elite and 88 B Pro; native linker aborts. No native executable runtime, mailbox trace, emulator PCM, or deadline test is claimed. No protection/region or stock sample modifications.

## Ordering and cancellation

1. Core0 chooses an already active sampled voice only for 1..220 frames, when the kit is on core0 and a bass job is on Core1. It copies the live 140-byte native voice snapshot before dispatch. The worker receives no pointer into the live kit. If launch is rejected, Core0 nulls the future and stays serial.
2. Core1 writes the private decoder state plus PCM, then executes the existing cross-core synchronizing barrier and publishes nonzero frame count. The snapshot/PCM fits at [4096,5120) of the first Core1 bass workspace. Its scratch lifetime is separated from any 808 prefix [0,4096); Core0 runs only other bass workspaces. DSP drive history is per voice. The worker's null dry route does not mutate shared mix buses or pans.
3. Core0 checks publication only at the source-selected voice's exact position in OH/RD/CR/CH summation. If pending, it renders the live state normally without waiting or reading worker state/PCM. If ready, it acquires, commits that exact voice once, and runs the matched panned/send add in canonical position. The worker cannot overwrite its future through bass rendering before existing stage 1 (published only after Core0 finishes the entire 909). Late future data therefore cannot contaminate the live state. Stage 1 is never dependent on worker completion; the existing bounded mailbox wait/takeover stops or joins Core1 before Core0 can reclaim the worker-owned workspace. No new blocking wait on Core0.
4. Worker computes the future before the existing 808-body dispatch and may wait on stage 1 before bass render. This absolutely can delay bass startup or redistribute deadlines adversely. The purpose of next loaded A/B is to confirm actual throughput benefit or reject, not to assert a performance gain from source structure.

## Evidence

test.sh compiles the patched real DSP and exercises full stereo kit, not a replica. In both -Os and ASan+UBSan: 8,000 published and 8,000 unavailable futures match all four raw PCM buses and byte-identical complete drum909_t state against ordinary serial render. Unavailable cases publish after serial render to prove late completion ignored; randomized drum activity and solo, panning glide, integral and fractional decoders, drive, envelope/counted mute and end-of-sample cases. This is sequential semantic evidence, not cross-core hardware or emulator coherence proof.

## Later guarded composition and actual replay

Independent exact EXP pot corrections/control span reciprocal compression in
`control/exact-app-fit.patch` unlocks a combined quarantined link. Native
strict 360 MHz offset 0, same source-pinned 922 events and synthetic S1
A/arp + Chicago B; each full 319,488-frame capture collected four settled
owner readbacks; checked NOR byte equality, zero skips, no mailbox errors
or giveup.

| Candidate | Elite peak µs / late | Pro peak µs / late | APP free Elite/Pro B |
|---|---:|---:|---:|
| Exact compression, no scheduler experiment | 13,032 /55 | 13,071 /44 | 548/560 |
| Exact compression + workspace future | 14,311 /86 | 14,494 /90 | 100/112 |
| Exact compression + stack future | 13,088 /60 | 12,013 /10 | 100/112 |

All remain 564 B above pool floor, same model guards; neither future adopted.
Stack shape is layered on the first transaction source by
`stack/stack-owner.py`. Core1 copies seed out of its currently unused bass
workspace into its local object and publishes its address, then starts normal
jobs immediately. Core1 necessarily owns a bass task when admitted, so cannot
leave the worker scope before the existing ordered bass stage (>909). Core0
never consumes the pointer after 909 and clears it before the next dispatch.
This leaves 1460 B of the untouched 4352 B Core1 user stack on both models
(high-water 2892), compared with 2484 B untouched for the workspace future.

Two identical Elite control executions have matching guard timing and bitwise
PCM (all 638,976 raw i16 values); cannot attribute candidate shift to repeat
randomness. Both workspace and stack models first differ at i16 index 140416
(stereo frame 70208). Stack all-zero onset is 512 frames later on both boards;
this is an authentic output mismatch, not corrected by fitted shifts. The
Elite instrumented native shadow arm runs both actual voice decoders when the
future is available, but always emits serial Core0 output. Across 218 real
published candidates: zero sampled voice state mismatches and zero raw
rendered voice-sample mismatches. Its native complete output still first
differs from control at i16 140416 although it emits no Core1 voice PCM. This
rules out wrongly committing the shadow's computed sample content as necessary
for the observed onset mismatch. It does not prove the first divergent final
bus or fully replay-exact event/clock behavior. The count labelled "missed"
also includes unarmed stretches matching the previous selected voice; do not
use it as an admission fraction. The instrumented 13,211 µs/51 result is NOT
reported as candidate timing. No shipping binary, actual device or flash writes.
