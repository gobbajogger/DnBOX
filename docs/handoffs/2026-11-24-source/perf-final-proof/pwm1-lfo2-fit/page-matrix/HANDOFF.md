# Exact compact pages, PWM2 joined

App gap in bytes (positive means free):

| Actual linked arm | Normal Elite / Pro | Guarded companion Elite / Pro | Companion |
|---|---:|---:|---:|
| PWM1 original | −13,008 / −13,000 | +280 / +288 | 13,650 |
| PWM1 compact titles/refs | −11,984 / −11,976 | +1,320 / +1,328 | 13,650 |
| PWM2 original | −13,208 / −13,200 | +92 / +100 | 13,662 |
| PWM2 compact titles/refs | **−12,184 / −12,176** | **+1,116 / +1,124** | 13,662 |

Eight identical ordered named page descriptors use 64 B total (4-byte title reference, one byte per column). High-bit tokens encode existing legacy engine parameters, the others encode a typed SuperJ6 slot, and 255 is the exact existing empty column. The original page construction is used unchanged outside those eight rows. On Elite, the joined describe_pages symbol falls 4232→3138 B; aggregating data, alignment and LTO yields **1,024 B actual app savings both models on PWM2, independently of relocating anything**. On PWM1 optimization saves 1,024 B stock and 1,040 B guarded. The genuine independent PWM2 receiver adds 200 B normal and 188 B guarded before page optimization; optimization funds this receiver and leaves 1,116 B actual limiting disposable margin. Per added page, catalog storage is 8 B plus the pooled title; e.g. sixteen four-slot pages with up to 11-character private titles represent 64 slots in 320 raw catalog bytes before link interactions. That is a catalog cost only. No count of new complete controls can be promised without their actual DSP, descriptors, restore/apply and packing costs, and the stock map still does not fit.

POOL +512 after reserve and sizes of .ram_text, .c1_text and XDATA unchanged in every arm. Guarded fixture still overlaps physical user Slot3 and is not authorized product storage. Retain internal recovery/font/palette. The linked frontier is in frontier.log: large 909 assets and packed Chicago curves are hot, the Miami transfer table/909 tanh/BREAK retrigger are live render or sequence consumers, not passive. Prior exact 2KiB bounded off-device compression trials (see bulk-asset-exact) estimate gross ~4,082 B drums / 3,658 B old Chicago tables; none includes validated on-device decoder, RAM or exact deadlines. Mixed literal pools require per-owner splitting; the 7,296 B entry is not a relocatable blob. No further large proven passive movement counted. This source representation is a sustainable fit lever without assuming lab storage or changing an audio path.

## Proof and reuse

Start at sibling PWM1+HELP exact source. For PWM1 only, apply page-matrix.patch. For the joined checkpoint, apply sj6-pwm2-lfo2/superj6-pwm2-lfo2-source.patch first, then page-matrix-after-pwm2.patch instead. Use the same paired normal-link-only and guarded companion drivers as the sibling handoff and read-maps.py. Both exact PWM1 paired arms and exact PWM2 paired arms each matched their own full page words, titles, unused columns, ordering, count and SMC digests over 8,800 pages contexts and 39,200 queries. In both PWM2 arms, the standard host SMC UI suite passes 8,839 checks; all 22 generated PNGs matched byte-for-byte. Guard actual independently linked pairs pass and swapped/corrupt cases fail with existing ASan/UBSan oracle (LeakSanitizer disabled for ptrace sandbox; no dynamic production storage). Source only. No native emulator, timing claim, map changes, device files or publication.
