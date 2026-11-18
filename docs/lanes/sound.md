# Sound lane

## Current owner checkpoint — 2026-11-18

This thread is the sole Sound production owner after consuming Dot Sound's frozen handoff. Preserve `lane/sound` and draft PR #973. The handoff archive SHA-256 is `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773`; its live head was rechecked as `b90faafd2de71063448beea67aa5d22938a5c64b`. Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` was merged in locally without conflicts; the existing lane history and files are preserved.

Latest user direction overrides the older implementation allowance: original SuperJ6 and Synth One engines are comparison references only. New code uses existing efficient DnBOX DSP and independently written behavior adapters. No external Codex or Actions jobs, PSRAM, device writes, main writes or force-push are authorized. There is one Sound writer.

The requested research logs are retained under RULES.md v2 rule 13; that rule and the current evidence-delivery request take precedence over the generic PR template's log exclusion. Firmware payloads and app installers are not new package contents.

The new [hardware-first research package](../../experiments/sound-hardware-v1/README.md) contains source, executable tests, independent review and retained failures. It is not wired into the production runtime. The [owner report](../sound/hardware-first-owner-report.md) records the complete evidence and gaps.

- Independent original-amplitude subset: 5,120 exact-timestamp endpoints pass the predeclared 0.001 threshold; worst error 7.95e-7. Shifted timing, linear contour and wrong velocity negatives reject. A real allocator defect was reproduced and fixed.
- Replacement RAM: both full part owners and existing selected FX remain simultaneously allocated. Candidate adapter plus stock chorus is 2,892 B inside the existing 3,720 B backend union; no pool growth is claimed or required by this partial core.
- Replacement flash: the compact reachable `-Oz` image is 553,420 B Elite / 553,412 B Pro. Physical overflow is 748/740 B; required-reserve shortfall is **1,772/1,764 B**. S6 remains FAIL, before the missing control/source/FX work.
- Callback: actual contract is 512/44,100 = 11.609977 ms. Candidate engine/event slice was measured, including event/control bursts and omitted render-counter work. Highest independent observed host maximum is 9.785132 ms. Full native callback, other simultaneous owners, stack high-water and xruns remain unmeasured.
- Synth One remains a silent production slot. The new common core does not match its four-pole transfer or VCA-before-filter topology. The historical original-code renderer remains reference-only; its existing fidelity results are not new-engine admission.

The production gates S1–S6 and common integration gates remain open or failing. No merge into consolidated is admitted by this checkpoint. Do not repeat the exhausted history-codec or compact-hall sweeps. The next work must reduce measured shared-code/adapter cost and independently qualify missing original behavior, with the whole simultaneous image accounted for.

## Preserved pre-handoff record

The current draft is PR #973 on `lane/sound`. This update restores the original generic Synth One renderer inputs and keeps its independent reference implementation separate from the experimental harmonic fixture.

## Checks on the restored source

- All 45 focused Python tests passed. Run `python3 -m unittest discover -s tests/host -p 'test_synthone*.py'`. Before restoration, three tests failed and seven raised errors.
- The portable bundle restored all 97 files. Its archive SHA-256 is `885f793f13845b74ded63151a65be75fdfadea79fc7696655397ea07bd498306`. The bundle and PCM manifest come from `f50d5c14`. The tuning fixtures come from `66b1ffef`.
- The pre-FX adapter rendered 576,000 frames across 12 scenes. There were zero differing channel words and zero render heap calls. The scalar control comparison checked 957,750 targets and found zero differing words.
- The generic factory comparison rendered 694 presets over 66,624,000 frames. There were zero differing or nonfinite words. Both outputs used the same FX implementation. All 1,388 output filenames, byte counts and SHA-256 hashes matched the historical PCM manifest. The commands in the portable input guide reproduce the test. [Factory results](../sound/restored-factory-replay.json) · [PCM manifest](../sound/v2-synthone-factory-pcm-manifest.json) · [Input guide](../../assets/test-inputs/synthone/README.md)

## Checklist

| Item | Status | Evidence or remaining work |
| --- | --- | --- |
| S1 | FAIL | The restored timing bench measured all 694 programs and missed 3,602 wall and 2,629 thread-CPU deadlines in 520,500 callbacks. All output words were finite. Profile the worst cases with the original floating-point settings. |
| S2 | FAIL | Historical harmonic A/B tests reported 492 bit-identical programs and 202 approximations. A current per-program spectral and level report remains open. |
| S3, S4 | FAIL | SuperJ6 control/FX coverage and comparison with the original captures have yet to be qualified on this lane. |
| S5 | FAIL | The replay covered 694 programs. Complete the independent control-to-parameter mapping check for the required banks. |
| S6 | FAIL | Last matched experimental pre-FX build exceeded Elite flash by 30,676 bytes and Pro by 30,668. Two-part pool margin was already 4,876 bytes below reserve before FX. Next measure retained code and independent voice histories against a matched stock build. |
| S7 | PASS | Case-insensitive whole-word Git scan under firmware, tests and web found zero hits on the restored source. |
| C1, C3–C6 | FAIL | Full firmware gate, independent review and overall checklist remain open. PR stays in draft. |
| C2 | PASS | The existing lane includes consolidated commit 89b40a93. No consolidated file was deleted. |

## Moved file

The experimental `firmware/src/dsp/synthone/scheduler/oracle/state.hpp` moved to `tests/fixtures/synthone-storage/oracle/state.hpp`. The generic renderer refuses to overlay any runtime source on its independent reference. Moving the fixture preserves that check and lets the normal renderer run against the pinned source.

## Timing and memory measurements

The callback benchmark and its original deadline contract were recovered from `55e1288d`. The untraced 48 kHz host test used 128-frame blocks. The full 694-program result was DEADLINE_FAIL. [Timing receipt](../sound/restored-callback-result.json)

At 44.1 kHz the four original ten-second delay histories alone allocate 7,056,016 bytes. In a fresh component test with pseudorandom input and a two-second delay, reducing the ring from ten seconds to 5.0001 seconds changed an output word at frame 220,528. An earlier minimum-size native build with link-time optimization still exceeded Elite XIP by 25,528 bytes and Pro by 25,520. The current build flags and effect behavior are preserved.
