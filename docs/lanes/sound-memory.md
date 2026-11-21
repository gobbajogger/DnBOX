# Sound memory on stock SMK-37 Elite and Pro

S6 remains **FAIL**. The regular native build links and packages a playable partial Synth One with either the shipped Juno backend or the experimental compact SuperJ6 backend. All five source variants below preserve the shipped UI and run with stock memory. The simultaneous full SuperJ6 and Synth One control sets, factory programs and independent original FX histories still require implementation and a new pair of links.

## Production INIT at the qualified source

The shipped Juno, Chicago, Miami and independently implemented Synth One INIT all link through the stock application. Both models use the same firmware source. The seven-file change on parent `933a6f40e8aca72607a258ab42ff21265489903d` reproduces code-only tree `39c6aea21022c93ee95de5a78236b3619b63bcdb`. Per-file pins, ELF sections, per-engine attribution, target ABI, builds and output identities are in the [production receipts](../../experiments/sound-hardware-v1/stock-fit-20261121/final-init-native-receipts.tar.gz).

| Stock model | Image bytes | Free XIP bytes | Free pool bytes | Pool above 8,192 | Free ordinary RAM |
| --- | ---: | ---: | ---: | ---: | ---: |
| Elite | 546,516 | 6,156 | 8,972 | 780 | 1,100 |
| Pro | 546,508 | 6,164 | 8,972 | 780 | 1,100 |

An independent reconstruction reproduced the ELF, BIN, FWSC and linker script for both models, eight files total. ELF symbol attribution for this source gives 1,764 bytes of Synth One functions and 48 bytes of named constants. Each Synth One part uses 956 bytes of the existing 3,720-byte backend union. Each shipped Juno plus chorus uses 888 bytes. The release image has 268 pool bytes above the required reserve plus safety margin. Two original instruments can require distinct wet reads from five seconds before a live delay edit. See the [per-program and independent-history measurements](../../experiments/sound-hardware-v1/synthone-delay/README.md) for the scope of this constraint.

The compact SuperJ6 candidate with a typed event explicitly retained for sizing leaves 1,136 B XIP on Elite and 1,148 B on Pro. It preserves the 780 B pool margin. Its base is the isolated AMP2 plus range-dictionary tree below. Its full preset importer remains outstanding. [Typed source and actual audio-block measurements](../../experiments/sound-hardware-v1/sj6-amp2/typed-entry.md)

A bounded four-ring delay tested in two real factory graphs used 2,748 B per owner. Eight slow controls passed. The two released factory histories failed all six preregistered measurements, reaching up to 18.735 dB absolute level error and 27.596 dB spectral error in the final mix. [Source, receipts and measurement protocol](../../experiments/sound-hardware-v1/synthone-delay/bounded-delay.md)

A 2,712-byte checkpoint design fit per owner and reproduced 176,400 feedback words with an ideal input provider. Prewarming reduced the predicted half to 893 µs. A subsequent delay-time edit changed every one of 2,048 tap words on one owner while the other owner stayed byte-identical. Reconstructing the requested history cost 23,211 µs, including 15,325 µs on the longer of two cursors. Both exceed the 11,609 µs deadline. [Target timing and live-edit replay](../../experiments/sound-hardware-v1/synthone-delay/prewarm-live-edit-proof.tar.gz)

Pruning unused feedback dependencies made the active cursor pair 660 B. Four existing 452-byte checkpoints bring the experimental state to 2,468 B per owner. On the same live edit, the target calculated and emitted all 2,048 original tap words for the edited part in 16,148 µs and all 2,048 for the other part in 1,228 µs. This still assumes access to the exact earlier dry samples and controls. [Split-cursor code and costs](../../experiments/sound-hardware-v1/synthone-delay/split-cursor-proof.tar.gz)

A bounded dry-source provider using eight bins per recorded note period occupied 224 B for saved data or 300 B including the active encoder. All four wet paths failed the 24 dB signal-to-noise contract on both original programs. The edited owner's long cursor alone took 14,752 µs on PI32v2. [Provider memory, accuracy and target timing](../../experiments/sound-hardware-v1/synthone-delay/bounded-periodic-source-proof.tar.gz)

The packed provider with its 224 B of stored records, count/end metadata, both historical reader states and both FX cursors measures 2,732 B, leaving 32 B in the current per-owner budget. Concurrently encoding new input while retaining both readers takes 2,800 B, or 36 B over. [Composed target layouts](../../experiments/sound-hardware-v1/synthone-delay/combined-storage-abi.tar.gz)

A separate finite low-pass stage passed 75 original control trajectories and all 252 existing dry-note comparisons. Its full applications occupy 546,676 B Elite and 546,668 B Pro, with 5,996 and 6,004 B of XIP free. The pool stays at 8,972 B, and ordinary RAM use stays at 97,204 B. This diagnostic adds 160 B to each stock application. [Physical results](../../experiments/sound-hardware-v1/stock-dry/live-cutoff-proof.tar.gz)

For the measured live edit, zero startup plus one 460-byte retained checkpoint provided both historical cursor starts. The full state with two synchronized workers, the source records and concurrent encoder uses 1,484 B per owner at 8 bins or 2,716 B at 64 bins. The larger representation leaves 48 B under the 2,764 B owner bound on this stimulus. It takes 14,720.716 µs for the edited warm half, and all 1,024 samples in each owner's final stereo output differ from the original. The ideal-dry control reconstructs the final words exactly. [Native layouts, scheduling and final audio](../../experiments/sound-hardware-v1/synthone-delay/dual-core-complete-wet-proof.tar.gz)

The sparse Chicago renderer with a shared general mixer linked at 546,148 B Elite and 546,140 B Pro, a 368 B application reduction on each. Its zero-XMod and nonzero-XMod costs moved in opposite directions. Both retained ordinary RAM usage of 97,204 B and 8,972 B free pool. [Native and host differential receipts](../../experiments/sound-hardware-v1/strict-timing/chicago-single-oscillator.tar.gz)

The two-tone note history stores 1,568 B per owner before encoder workspace. It missed the four-tap wet-audio criteria on both original programs. Supplying the actual individual voice phases for a separate offline fit required another 29,635,200 B per owner plus 4,160 or 16,448 B of fitted coefficients. At 64 bins, one of eight wet taps passed. These measurements leave the production S6 result open. [Source size and quality measurements](../../experiments/sound-hardware-v1/synthone-delay/note-source-and-voice-phase.tar.gz)

The original voice code can resume from **216 B per voice, or 1,512 B for seven slots**, on the PI32v2 ABI. Across 2,087 forced resets and restores per owner, both complete six-second traces matched all 5,292,000 original binary32 words each. Erasing oscillator phases or filter histories changed more than 2.4 million words on each program. The rest of the scheduler, shared random generator, control smoothers and effects stayed live. This qualifies one part of an independent historical reader. Its remaining state, complete native replay cost and S6 fit are open. [Voice checkpoint, positive and negative controls and target layout](../../experiments/sound-hardware-v1/synthone-delay/original-voice-checkpoint.tar.gz)

Two complete independent historical sources reproduced the actual stereo delay inputs. Their largest saved records were 2,044 and 2,060 B before compression. Two useful archived positions take 1,205 B on the first program and 1,788 B on the second using the best measured lossless format for each. These counts exclude the decoded destination and active source and feedback workers. Regenerating the edited owner's 17,556-frame long input from one snapshot used 18.409–32.171 ms of host thread CPU. The production pool remains 8,972 B free. S6 remains FAIL. [Original input and source workload](../../experiments/sound-hardware-v1/synthone-delay/independent-original-input.tar.gz) · [Lossless sizes and target decoder](../../experiments/sound-hardware-v1/synthone-delay/lossless-source-records.tar.gz)

The selected Synth One part can reuse its retired Chicago insert's 4,096-byte buffer. With 2,764 bytes unused by the current Synth One backend, this gives 6,860 conditional bytes per part. Filling both buffers preserved the complete 36,420-byte project and all 92,672 stereo frames through real bank changes, notes and rendering. All 12 Chicago algorithms matched their pristine outputs for 14,336 frames each after a nonzero overwrite and cold re-selection. Rejected and pending requests retained the old owner. Both production ELFs reserve 8,504 bytes for the two insert objects and leave 8,972 pool bytes free. The complete historical reader still needs a live working-set and timing measurement. [Ownership fixture, target layouts and receipts](../../experiments/sound-hardware-v1/synthone-delay/selected-insert-ownership.tar.gz)

Both target links contain two 5,120-byte bass render workspaces. Part A must retain the first 4,096 bytes of its workspace for the 808 metal split. Part B can reuse its whole workspace when Synth One is selected. Together with the inactive Chicago insert and backend tail, the conditional capacities are **7,884 B on part A and 11,980 B on part B**. A common 7,628-byte layout holds two decoded source records, their two compressed anchors, the 1,024-byte feedback workers and a 460-byte original feedback checkpoint. A clean restore overwrote the two decoded slots before reconstructing them from the retained archives. Both owner assignments preserved the entire 36,420-byte project and 92,672 stereo frames. A forced production split exercised all four 808 metal planes in 352 renders. The unrestricted host planner chose zero splits on the same stimulus. [Layouts, firmware transitions, restores and complete comparisons](../../experiments/sound-hardware-v1/synthone-delay/selected-historical-reader-storage.tar.gz)

Four independent sources seeded from that firmware readback regenerated every requested stereo input sample. Supplied with only those ranges, the two target feedback workers reproduced all 512 stereo output frames, 2,048 historical taps and 2,048 new writes on each part. Feedback alone took 9,406.277 µs for the edited part and 887.723 µs for the other part. The original pan, bitcrusher, phaser and input-filter stage now runs from a single embedded 624-byte host object with zero heap allocations. Its PI32v2 layout is 492 B. The full source voice and scheduler are additional. The complete effects still lack a real-time native fit. [Independent source-to-feedback and active workspace](../../experiments/sound-hardware-v1/synthone-delay/selected-historical-reader-storage.tar.gz)

All 30,167 original active voice calls in the requested histories reproduced all 120,668 captured reference words. In a deliberately optimistic target measurement, all arithmetic helpers ran at zero cost. Even charging only instructions in the edited program's voice caller, with free subroutines and ideal division across both cores, the two historical cursors require 41,892.549 µs together. Replaying the literal original voice path cannot meet the audio deadline for this edit. [Inputs, target accounting and reproducible checks](../../experiments/sound-hardware-v1/synthone-delay/original-voice-free-math.tar.gz)

## Earlier candidate images

These local stock builds use `BOARD=smk37 MODEL=elite|pro sh build.sh`, default development identity 900 and the default empty BREAK bank. The physical application area is 552,672 bytes. Each image includes the four-byte end marker. Each row has exit 0 for both models. All trees derive from `0cddaeb822458fdf4ed3b0b9035fcf22248fda10`.

| Playable partial Synth One with | Code tree on the PR parent | Elite image / free XIP (B) | Pro image / free XIP (B) |
| --- | --- | ---: | ---: |
| Shipped Juno engine and chorus | `1e18f1b0706a8c1ec17f0939a486382b69111c9b` | 548,512 / 4,160 | 548,504 / 4,168 |
| Compact Juno facade and core | `2ca530ce1031327a1098f120d1745cc6294a85cc` | 550,808 / 1,864 | 550,800 / 1,872 |
| Compact core with physical-range dictionary | `731158c6ad7b30addcb6f635f4eca4cbc56bd941` | 550,744 / 1,928 | 550,732 / 1,940 |
| Compact with direct AMP2 runtime fields | `bf5aed21ca2649c2fce367131afd36565b92a436` | 551,224 / 1,448 | 551,216 / 1,456 |
| Compact AMP2 with expanded physical-range dictionary | `18f29e2ce3d83d034a76c6ca6ae79962c12d2350` | 551,152 / 1,520 | 551,140 / 1,532 |

In all ten links ordinary RAM use is 97,204 of 98,304 bytes, pool use is 335,092 of 344,064, and XPOOL use is 9,216 of 9,216. Raw pool free is 8,972 bytes. This leaves a 780-byte margin over the 8,192-byte reserve, or 268 bytes beyond the additional 512-byte gate requirement. No target runtime or stack high-water measurement was made.

The dictionary saved 64 bytes on Elite and 68 bytes on Pro in this composition. The existing host boundary comparison was rerun with the candidate's actual shared-math and ABI headers. The 944 float boundary cases, four oscillator-mode cases and 576 renders across four rates passed, with identical complete owner state and audio over 31,488 frames. The pulse-width mutant failed. This change alters macro validation; its stock hardware timing was not measured.

Adding direct AMP2 fields to the compact core cost 416 image bytes on each model. Its expanded 11-range dictionary saved 72 bytes on Elite and 76 bytes on Pro. AMP2 owners retain the 24-second amp decay/release bound and generic owners retain the 16-second bound. Both owner profiles passed their own 944 boundary cases, four mode cases, 576 complete-state/audio comparisons and wrong-range negative. These two additional compositions measure deployment size of the direct runtime adapter. Complete preset ingress remains unqualified.

## Engine-specific measurements

The first table uses the shipped-Juno/S1 composition. Clang compiled the owner sizes for the PI32v2 target. The drum/BREAK entries are their actual image pool symbols. The firmware allocates two 5,184-byte owner objects in `original_bass`. Each contains one 3,720-byte backend union plus shared ownership state. Per-owner figures describe alternative union members. Independently, the two 5,120-byte workspaces are physically retained in POOL and XPOOL. The stock shared bus retains 18,316 bytes in `fx` and 158,400 bytes in `dly_buf`. The selected insert arenas `original_fx` occupy 8,504 bytes across both slots.

| Engine | Backend per owner or fixed drum/BREAK object (B) | Named functions in flash (B) | Named read-only data in flash (B) |
| --- | ---: | ---: | ---: |
| Chicago | 3,492 | 29,460 | 34,453 |
| Miami | 3,720 | 18,280 | 5,327 |
| Shipped Juno with chorus | 888 | 2,340 | 0 |
| Partial Synth One | 1,048 | 2,638 | 0 |
| 808 | 3,680 | 342 | 11 |
| 909 | 7,612 | 3,836 | 32 |
| BREAK | 6,772 | 1,646 | 0 |

The compact SuperJ6 backend plus chorus uses 2,892 bytes inside the same union. Its dedicated functions/read-only data measure 3,498/184 bytes in the initial compact composition and 3,514/103 bytes with the dictionary. The totals in each row match across both models.

Flash attribution counts disjoint, nonzero ELF `.text` symbol ranges with an engine prefix. Function symbols are counted separately from named constant bytes. Compact Juno also owns the source-verified local `params_valid.bounds` and `params_valid.field_range`. Data merged into anonymous literals, inlined code in generic callers, shared DSP, padding and the remaining firmware are left unattributed. The shipped-Juno/S1 images have 450,147/450,139 bytes outside those named ranges. These counts are lower bounds for engine-specific storage. A zero means that no named constant was attributed by the stated rule.

## Reproduction and limits

The proof packet contains the model logs, linked ELF symbol/section tables, exact source and output SHA-256s, physical linker script, PI32v2 ABI probe, readback and symbol-counting code. The JieLi Clang and linker match their pinned digests and `clang --version` returns 4.0.1. The three AC79 V1.2.14 packaging files match the individual pins in `tools/build.py`. Rebuild with those verified inputs for each model in turn. Use `common/bin/objdump -t -h build/<model>/x0x.elf` and the packet's `link_attribution.py` to reproduce the flash counts.

Shared sends and the partial synth inserts exercised in these images do not supply four independent ten-second delay histories. The earlier original renderer allocated 7,056,016 bytes for those four buffers at 44.1 kHz; its warm shortened-ring oracle diverged at frame 220,528. Acceptance still requires a stock-fitting independent design, matched original audio and full callback qualification.

[Native measurement packet](../../experiments/sound-hardware-v1/stock-fit-20261121/native-receipts.tar.gz)

## Additional native and original comparisons, 2026-11-21 UTC

- An isolated Chicago filter change removed redundant nonfinite checks and shrank both stock application images by 96 B. The bitwise control and failure-path comparison covered 280,000 cases, including 3,384 mid-span failures. Elite measured 546,420 B and Pro 546,412 B. Both retained 8,972 B of free pool. Its strict timing qualification remains separate. [Source, differential and link receipts](../../experiments/sound-hardware-v1/stock-fit-20261121/chicago-finite-proof.tar.gz)
- Six additional isolated SuperJ6 controls cover the pre-note base waveform and pulse width. Of 1,280 authored layer records, 1,079 have sufficient fields and 201 square records omit required PWM state. Both layers qualify on 474/640 programs. The combined typed entry measured 551,840 B Elite and 551,828 B Pro, leaving 832/844 B free. That misses the 1,024 B XIP reserve by 192/180 B. Pool margin stayed at 780 B. [Bank, replay and model receipts](../../experiments/sound-hardware-v1/sj6-amp2/oscillator-prenote.tar.gz)
- The current INIT filter holds 2 kHz and 0.9 resonance. Its public 131,072-frame held note remained finite. A fixed-coefficient sweep tested 21,987 points. Alternating the isolated cutoff to extremes every four frames produced nonfinite samples at high resonance. A future live cutoff entry needs a qualified transition law. [Stability reproducer](../../experiments/sound-hardware-v1/stock-fit-20261121/s1-live-filter-stability.tar.gz)
