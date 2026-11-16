# Metadata-only compact Scheduler, bounded source candidate

SynthOne is intended to replaceDrone, but this does not change firmware, saved kinds, presets or defaults. Both engines must ultimately preserve ALL original controls, modulation, FX, seq/arp and pad/visual workflows. The current direct-event Scheduler is still only arp-off/empty-pending PRE-FX. No full-port or product acceptance.

## Actual change

The final caller-owned Scheduler cold-validates a compact-v1 container,52 NULL morph-row payloads with original4097-count/band/shape topology, exact13 band words, real original sine,128 tuning values,150 legal controls and sample rate44100/48000/96000. It validates all candidate inputs before retiring live state. The original init_graph and source note arithmetic are unchanged; metadata descriptors are caller-owned and their morph tbl pointers stay NULL. Only the runtime note backend calls the already reviewed compact C reader. The raw original oracle, raw/direct APIs and original generation checks remain separate.

Containers are borrowed immutable bytes, not copied/decompressed full assets. Sine and tuning are also borrowed. Parameters and Tables metadata need only survive initialization. No metadata pointer is retained into a temporary Tables object. Canonical-content/source authentication remains external; structural admission cannot detect every finite wrong sample. No heap allocation/cache is added to audio/event actions.

Whole-final-owner alias checks reject packed, Tables, controls, tuning and sine intersecting any member of the final Scheduler. Prefix output spans cover frame offset and reject owner/payload/sine/tuning intersections. Cold rejects leave the complete old owner unchanged. As in the inherited direct reader, process failure caused by violating immutable/mapped/lifetime/internal-object contracts has no full-frame/block rollback guarantee: earlier output/state and source wtpos/inc may already change. Do not mutate the borrowed asset or internal graph concurrently.

## Verification boundary

All54 inherited direct-event cases run through compact runtime versus unchanged distinct-row raw oracle. Only physical table addresses are normalized in snapshots; source fields/descriptor metadata/PRNG/filter/control/voice/held-note histories and every PCM output word remain exact. Additional cases test69 whole-object transactional admission rejects, simultaneous instances/temporary metadata lifetime, unaligned container views, output/owner/payload/tuning/sine aliases and actual aliased logical48/49 release reads. O2/Os each60,530,530 checks,19,602 frames,20,850 snapshots,660 events,211,224 actual word reads;54 inherited+9 metadata cases; action heap calls0. This is bounded differential coverage, not all possible presets/input histories or a whole engine audio-fidelity verdict.

Default LSan fails under ptrace. SAME nonrecovering ASan/UBSan/float-cast binary passes with leak detection disabled and empty stderr. No leak-pass claim. Legacy raw CLI remains valid without a compact argument; --compact selects the new fixture and is incompatible with other specialized fixtures.

Genuine pinned pi32v2 compiler verifies23 runtime objects and layout: Scheduler/RuntimeState9988B, State9552, Ports396, seven note objects3108,53descriptors1484, compact binding12B align4. These are objects/layout only, not executable/firmware link, stack/CPU, safe asset placement or total-product fit. Existing retained full-engine diagnostic fails platform services and soft-double policy; no stubs/floatification or protected flash borrowing is authorized.

## Reproduction

Use the existing runner with exact --experiment-root/--controls-root/--master-root/--tables input dependencies, optional --compact pointing to the private165292B canonical packed fixture, --variant o2/os/sanitize and a new --build-root. It regenerates the unchanged oracle and direct reader from pinned original source. The compact note derivative is a lexical backend/type/namespace substitution of the accepted direct-reader note, not new DSP equations. Private fixtures, SDK/toolchains/binaries and reports are not publication inputs.

Next: independent affected review, reviewed non-main source preservation, real retained compact engine link/resource/numerical policy and CPU/whole-product qualification; separately port actual original seq/arp/FX/modulation/presets/UI/pad behavior and fullSuperJ6 semantics without dropping anything. SavedDrone data requires explicit versioned migration/recovery policy, never interpretation as SynthOne bytes.

Independent R2 correction: metadata and packed frequency-band words are each compared directly to the original binary32 words, not numeric equality or agreement with each other. Three regression variants (metadata-only/payload-only/both negative zero at band0) reject without changing the live owner. All raw oracle and DSP arithmetic remain unchanged.
