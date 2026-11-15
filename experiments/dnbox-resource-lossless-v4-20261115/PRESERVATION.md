# Reviewed lossless cold-curve storage, V4

This is source preservation, not a competing integration branch or an installable firmware. It does not modify any shipping tools or runtime at this preservation path. The source change is an overlay of tools/gen_drum_samples.py on exact #937 ffd31a735acc5721d1f90a30fc1ceb283aad6fc5 plus the previously reviewed lossless-resource and frozen PRETRANSPORT remote inputs.

All 19 x 128 original IEEE binary32 EXP words are retained exactly. A fixed binary32 exp2 polynomial predicts each word; a signed row offset and four-bit residual reconstruct the original word. The output is not approximate DSP. ADPCM samples and the non-EXP table prefix are unchanged. Generated C is standalone and has no libm/double/heap/cache dependency. Valid-domain row/pot and fp-contract=off requirements remain; this is not an arbitrary-input or alternative-FP-mode API.

The original README is retained byte-for-byte as a dated producer record. Its review-pending wording is superseded here: a non-author independently verified all frozen inputs, rebuilt O0/O2/Os and nonrecovering sanitizer exact-word tests, checked standalone/multiple-TU linkage, exercised corruptions, checked the sole generator difference across 5,445 source files, and independently reproduced both real native ELF/BIN outputs byte-for-byte. Source and bounded native attribution were accepted. No target timing or whole-product acceptance follows.

Original EXP word stream SHA-256: c8c351223b0c805e68f88c917c5ed0a8b4983277681e2a1b43251627ffbd3faf (2,432 words).
Producer unchanged setter/stereo witness: 1,880,320 bytes, SHA-256 f34ed29007f532b70238a840be78ce791d6459ab70a74adddcf00edd4f5a572e. Audio is not included here.

Exact PADCAL=1 PRETRANSPORT native composition:
- Elite: image 544,472 B, flash free 8,200 B, only 8 B above the required 8,192 B reserve.
- Pro: image 544,460 B, flash free 8,212 B, only 20 B above that reserve.
- Pool free 8,332 B; ordinary RAM free 284 B. Both actual maps/commands were checked, no soft-double/undefined symbols.
These are extremely thin bounded margins, NOT final product headroom. Latest USB/BLE transport, polished UI, full SynthOne and full SuperJ6 are not in this closure. No arithmetic savings sum is called their fit.

The reconstructed lookup has more control-access instructions than the prior decoder (53 decoded instructions, including 17 FP and four conversions, versus 21/zero FP). No target cycle/no-slower, real audio timing, stack-high-water, hardware, browser, installation, release or publication qualification is claimed. Those gates remain open.

Reproduce standalone valid-domain proof from this directory:
    cc -std=c99 -O2 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror test_exp_exact.c -o /tmp/dnbox-exp-proof
    /tmp/dnbox-exp-proof
The generator requires the exact admitted source tools/sampleio.py and original assets/909 inputs, outside this source-only preservation folder. Do not run it as an unbound replacement or silently regenerate with unverified dependencies.

No SDK/toolchain content, native executable, firmware image, audio, private capture/report, credentials or device write is included in this public subset. Full private provenance and native build artifacts are retained separately.
