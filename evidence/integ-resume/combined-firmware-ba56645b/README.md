# Combined stock firmware software qualification

The tested firmware is `ba56645b2142b134be3aeee2236406e1c95b2bde`, tree `3093322cfeea194e4d9b190a989fe9ef4d01e874`. It contains consolidated `1d727e2ba118f0c85b20f3ed241ec133e4882d7e` with zero deleted consolidated paths. The exact paired Nightly source is `31a92a944c9303940d5409cd213dedba1d47c793`. [Nightly qualification](https://github.com/gobbajogger/DnBOX-nightly/blob/175c3d3cf6a71fb8f7d312dac82e3898ad75f01c/evidence/integ-resume/combined-31a92a94-ba56645b/README.md)

## Stock fit and input identity

The [Sound checkpoint](https://github.com/gobbajogger/DnBOX/tree/07a065c9232666c9bedbc87ca98cc955caaaec1f/evidence/sound-resume/normal-wide-stock) pins Integration source `1c46c6fc`. The matching [overlay](verified-sound-overlay.patch) shares the two square-layer calculations only when both layers and every voice layer state are equal. It compiles stock WIDE FXBUS at `-Os`. The original Sound proof records bit-identical 1,800-block PCM and state comparisons, including mixer and delay state. Input hashes, stock application ELF and FWSC outputs matched the [Sound pins](sound-overlay-pins.json) before the recorded short results were reused.

| Model | Candidate package | XIP free | Pool margin | Normal wet WIDE, 8 s | Chicago/Miami, 8 s |
| --- | --- | ---: | ---: | ---: | ---: |
| Elite | [Stock Elite FWSC](DnBOX-lane-integ-resume-0130-ba56645b-stock-elite.fwsc) | 1,028 B | 1,244 B | 10,893 µs | 9,219 µs |
| Pro | [Stock Pro FWSC](DnBOX-lane-integ-resume-0130-ba56645b-stock-pro.fwsc) | 1,036 B | 1,244 B | 11,561 µs | 8,793 µs |

Both builds satisfy the 1,024 B XIP and 512 B pool floors. All four bounded scenes recorded zero late halves and xruns. WIDE ran in real mode-2 chorus. The complementary scenes used the stock Chicago/Miami banks, both drum kits and loaded BREAK. Their saved guest RAM shows positive Chicago control updates and table cache state, two configured Miami unison lanes and active modulation cache. The endpoint Pro sample has both Miami lanes active. [Elite build](elite-build.log) · [Pro build](pro-build.log) · [Elite Chicago/Miami readback](elite-chicago-miami-readback-8s.json) · [Pro Chicago/Miami readback](pro-chicago-miami-readback-8s.json)

## Software protocol, BREAK assets and compact programs

Both actual stock images passed the production Remote exchange in strict emulated USB MIDI. The source-owned 909 scalar reads 75, applies 76, reads revision 2, undoes to 75 at revision 3, closes the lease and rejects the stale write. The exact host checks cover the real editor queue and saturation, panel edits and boot incarnation. [Elite USB receipt](elite-native-usb.json) · [Pro USB receipt](pro-native-usb.json) · [Production host checks](dnr2-scalar-host.log)

The [five-zone distribution archive and offline readback](../qualification-0130/stock-bundle-manifest.json) remain the approved software inspection scope. This is tied to the archive’s named stock packages and stock partition metadata. The stricter owner direction is software-only and no physical qualification is recorded. The compact-program LeakSanitizer result is carried from its untraced offline guest. The [exact closure](compact-program-untraced-input-closure.log) checks all ten input blobs and retains the prior real leak and corruption controls.

## Heavy host workload

The existing archived host probe exercised 16 ordered bass pairs with two control sweeps each, all five parts, all 22 drum rows and up to eight offered notes per channel. It completed 51,712 blocks and 600.375 simulated seconds across separate cases. The muted Bass B negative failed its coverage condition as expected. Nine CPU and nine wall deadlines missed. CPU and wall peaks were 26,096.914 and 26,102.277 µs. The fixed arithmetic control on the same CPU separately missed once with a 15,377.340 µs thread peak. The observed host workload still fails the strict threshold. [Host timing](all-engine-host-stress.json) · [All cases](all-engine-host-cases.jsonl) · [Host control](all-engine-host-fixed-work.log)

## Canonical firmware Local gate

The unchanged firmware gate on `ba56645b` exited **1**. Pool accounting, setup, clean tree, no-loss, limits, both stock builds, flashmap and memory budget passed. The host suite reported 476 PASS, 51 FAIL and 2 NOT_RUN. The internal sequencer passed, with external TB-3PO A/B skipped. The required reference preflight also identified the excluded 808, 909 and TB-3PO inputs. The owner’s current scope excludes these external comparisons. [Exact terminal evidence and registration inventory](canonical-terminal.json) · [Scope preflight](original-engine-scope-preflight.log) · [Canonical flashmap and budgets](canonical-flashmap-and-budget.log)

The UI geometry stage failed three cases in its 44-test negative suite. The current flat strip lacks the expected instrumented label hook, and removal and duplication mutations fail to elicit the expected rejection. Both captured gate log copies end with `Drone mentions: 0`. The process exit is recorded separately because the gate's final text did not reach either copy. [Raw receipt and archive digest](raw-gate-retention.json)

Focused diagnosis retains the original failures. Correcting only the simulated audio deadline in a disposable PANIC fixture restores all 36 assertions. The stock pan test reaches all 22 bindings and then fails its fixed descriptor total, which moves from 197 to 211 with a 14-control increase in the retained 808 KIT table. [PANIC test clock control](panic-fixture-reset-clock.log) · [Consolidated and candidate pan](pan-consolidated.log) · [Candidate pan inventory](pan-candidate.log)

FWUI #992 remains separate at `4df0102f742417ad27e7f3510c2d89d0b430b7f4`. Its preserved historical gate has 333 host passes and 32 failures and no terminal Local verdict. The [separate inventory](separate-fwui-992-gate-inventory.json) is unchanged in scope.

The [latest branch-only stock Remote catalog and layout](https://github.com/gobbajogger/DnBOX-nightly/blob/2d160b79223ad5b814c6dd2a51548c7536937103/evidence/integ-resume/stock-native-ba56645b-21a32d8/README.md) independently qualify with this firmware and composed Nightly `e3053cd7`. The exact full Nightly gate failure is retained at its earlier `31a92a94` input.

Both exact packages passed direct format, stock flash and OTA validation. The Elite package is 651,732 bytes, CRC32 `88e409e7`. The Pro package is 651,730 bytes, CRC32 `c7318ae1`. [Elite package verification](elite-package-validator.json) · [Pro package verification](pro-package-validator.json) · [Full host registration list](canonical-all-529.json)

The separate [direct UI judge](ui-judge-direct.json) rendered and walked 1,824 exact host frames. It exited 1 after reporting 4,680 instrument findings, 315 missing presence checks, 162 missing glyph checks and zero damaged glyph checks. Those are instrument counts. Individual product findings still need triage.

## Strict 600-second firmware playback

All four software runs passed the strict 360 MHz deadline on the exact stock firmware. Each run used 605,000 guest milliseconds and verified nonzero audio in all 600 scored PCM seconds after the saved-project PLAY event. Every timing counter covers the complete boot-to-end window. [Exact receipts and local archive digest](strict-stock-playback-600s.json) · [Emulator tick and firmware timer interpretation](strict-emulator-clock-and-scope.md)

| Stock model and saved-project scene | Peak | Completed halves | Late / XRUN / skipped |
| --- | ---: | ---: | ---: |
| [Elite WIDE A, Synth One B](elite-wide-600s.json) | 10,893 µs | 52,086 | 0 / 0 / 0 |
| [Pro Synth One A, WIDE B](pro-wide-600s.json) | 11,561 µs | 52,086 | 0 / 0 / 0 |
| [Elite Chicago A, Miami B](elite-chicago-miami-600s.json) | 9,321 µs | 52,086 | 0 / 0 / 0 |
| [Pro Miami A, Chicago B](pro-chicago-miami-600s.json) | 9,024 µs | 52,087 | 0 / 0 / 0 |

Both paired scenes exercise all 22 909 and 808 rows, both bass owners and the populated BREAK slot. The WIDE scenes exercise real mode-2 chorus and the Synth One highpass/sub controls. The Chicago/Miami terminal readback verifies nonzero cache indices, two Miami lanes with two simultaneous activities at the Pro endpoint, and positive engine activity counts. It does not instrument a continuous voice high-water mark. These four passing emulator runs retain their exact scope. The all-engine maximum-note host workload and both canonical Local gates remain failing.
