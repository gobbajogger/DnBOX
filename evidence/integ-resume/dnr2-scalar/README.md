# Software-only scalar Remote producer

The bound fixture answers DNR2 HELLO and exercises one independent integer control through the production `ed_service` dispatcher. The shipped stock configuration leaves the candidate unbound. Stock INFO still returns 25 bytes and stock HELLO returns zero.

## Focused verification

Run `sh tests/host/run_dnr2_scalar_dispatch.sh /absolute/path/to/pinned-remote-checkout`. The optional JavaScript step uses the Nightly Remote module and its DOM fixture at [995d5493](https://github.com/gobbajogger/DnBOX-nightly/commit/995d5493386fad889ca8106758567c9fe80a47d5). All consumed module and fixture files were clean against that commit. The fixture confirms Apply and guarded Undo with exact unit, client, epoch, request, control, revision and value fields. The readbacks are 75 → 76 → 75, at revisions 1 → 2 → 3. The real host engine fixture independently observes the corresponding 909 MIX Level project mirror, queue admission and consumed gain. It checks negative results for saturated queue and A–B–A panel edits, while the wire fixture checks malformed frames, bad addresses, duplicate requests, expiry, project protection and maintenance admission. The existing project editor/sample protocol also passes.

The JavaScript and real-engine proofs are complementary. The DOM fixture runs a test scalar, while the real engine fixture injects the queue and selected external-writer hooks at the software boundary. No application or catalog claims production admission.

## Stock fit

The stock input is [e29c99ec](https://github.com/gobbajogger/DnBOX/commit/e29c99ec2cc610e5af3de575e85771f666b239f4), app SHA-256 `650b8dced69b28dea1f549fb87fc0610e5afc3b9aec66c85bd5cedc56f246f45`. The cached relink compared .text, .ram_text, .c1_text and .data byte for byte. Adding the scalar core and a synthetic acceptance predicate increases the application from 551,444 to 552,468 B in the 552,672 B Elite window. The remaining 204 B is 820 B below the required reserve. The estimate excludes the actual dispatcher, stock incarnation source, real queue integration and writer notifications. Both unbound boards have the same nonempty preprocessed unity output as the stock source, so the stock package and offline five-zone evidence were reused.

With the pinned JieLi toolchain, compile `native-fit.c` for pi32v2 with `-mcpu=r3 -mfprev1 -Oz -flto -ffunction-sections -fdata-sections -fno-builtin`, the repository root include and toolchain standard include, into `dnr2-native.o` in a disposable output directory. Run `python3 -B relink.py OUTPUT STOCK_CHECKOUT TOOLCHAIN`. The stock checkout must contain the exact existing Elite build objects. See [cached native output](cached-elite-relink.log) and [software transcript](software-regression.log).

## Production qualification

Production admission needs an exact combined Sound/FWUI link with at least 1,024 B free XIP and 512 B pool margin, a qualified boot identity, the source-owned catalog and checked queue admission. Every external write to the addressed value, including project adoption and local edits, must invalidate prior revisions. The candidate conservatively refuses commands while the existing boot-flash busy predicate holds, including active playback. Product behavior during that state and OPEN replay after intervening commands need review before activation. Keep physical hardware and release actions out of this qualification.
