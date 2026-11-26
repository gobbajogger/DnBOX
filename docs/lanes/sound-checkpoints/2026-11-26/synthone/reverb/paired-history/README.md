# Full-rate source RevSC: paired history storage and budget frontier
Host diagnostic using exact 264,728-frame original Dark Theater Organ room send including 128-frame preroll. Full 44.1 kHz source RevSC algorithm, eight modulated line addresses, original cubic read and interpolation, feedback, pole and stereo topology unchanged. No gain/phase fit, changed preset, target borrowing or device work.

Two chronological neighboring Float32 ring writes become (a+b)/2 and (a-b)/2. Unpaired final addresses use zero as partner. A per-line pending register preserves the newly written even sample while its odd neighbor is still the old ring generation; no untracked overwrite/reencode or intersample delay is introduced. Reads reconstruct normalized sums/differences for the four original taps independently. This is not channel M/S; the pairs are consecutive time samples within each original history.

- Double proof (not a codec proposal): all 1,059,197 completed pairs reconstructed the original input Float32 words exactly, and full room return was bit-for-bit original. All tested codec variants produce the same hash of every evolving original physical line read position. Verified round-trip energy: sum 229,458.287, difference 641.529, **difference energy −25.535 dB** vs sum; peaks 2.3881 and 0.9343. Do not infer differential peak from average energy.
- 16 on sum uses either original BF16-style E8M7 (broad exponent) or narrow 16-bit E5M10 (bias 22, more mantissa in usable measured range; not IEEE half's default bias). 12-bit E5M6/8-bit E4M3 use the documented existing range. Region size includes pending words and alignment; add 440 B observed host RevSC owner state. No saturation in any tested arm. Underflow counts remain in result.json.

| Sum / difference | Ring payload including pending | Whole raw residual | Last second raw L/R | Room spectral L/R |
|---|---:|---:|---:|---:|
| Stateless full-rate BF16 baseline | 49,392 B | −54.934 dB | −51.77 / −51.54 dB | 2.457 / 2.899 dB |
| Paired BF16 / narrow 12 | 43,312 B | −53.106 dB | −49.31 / −49.19 dB | 2.893 / 3.281 dB |
| Paired BF16 / 8 | 37,152 B | −52.259 dB | −48.62 / −48.34 dB | 3.838 / 4.305 dB |
| Narrow 16 / narrow 16 | 49,472 B | −71.144 dB | −67.47 / −67.27 dB | .066 / .077 dB |
| Narrow 16 / narrow 12 | 43,312 B | −70.266 dB | −66.78 / −66.49 dB | .080 / .097 dB |
| **Narrow 16 / 8** | **37,152 B** | **−59.294 dB** | **−55.90 / −55.54 dB** | **1.536 / 1.780 dB** |

Narrow 16/8 is the smallest tested arm passing the original isolated-return spectral threshold; at −.00037/−.00038 dB late level and no room-phase change it improves over direct stateless BF16 with ~12.24 KB fewer rings. Difference smallness didn't make it disposable: dropping it to 6 bits at 34,056 B sends spectra to 8.306/8.590; the 14/4 arm at 27,880 B fails 16.185/16.550 with ~667k difference coefficients rounded to zero. The 17-bit/pair trio already reaches 26,328 B rings (+440 B host state) but best 11/6 still 9.829/10.300 dB. No successful arm fits the current 2,100 B room or a hypothetical 26,880 additional plus current 2,100 B combined envelope. Speaker/UI and multi-instance memory are not proven available, and no allocation was changed. The simple diagnostic reader loops bits for clarity, not a native feasibility/CPU assertion.

Reproduce by passing sibling matched render-tap directory to replay.sh. Only source and numeric traces/receipts are retained. No manifest contains a preset pack, physical binary, generated render tapes, or raw history.
