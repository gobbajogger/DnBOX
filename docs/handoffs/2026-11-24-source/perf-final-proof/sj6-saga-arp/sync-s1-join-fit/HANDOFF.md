# Exact combined Synth One oscillator edits and SuperJ6 Saga ARP

Source-only integration on the exact compact PWM2 + sync + owner-pointer checkpoint; both original feature patches applied forward without conflicts, including their shared UI regions. The individual Synth One exact PREIMAGE/POSTIMAGE checks all passed. The combined patch with exact 15-file pre/post checksums applies cleanly forward/reverse. This is not a unified release.

| Exact arm | Normal app free Elite / Pro | XIP growth vs base Elite / Pro | Guarded app free Elite / Pro | Guarded growth vs base Elite / Pro |
|---|---:|---:|---:|---:|
| Base compact sync + owner | −13,100 / −13,092 | 0 / 0 | +252 / +260 | 0 / 0 |
| Synth One oscillator only | −16,024 / −16,012 | +2,924 / +2,920 | −2,664 / −2,652 | +2,916 / +2,912 |
| Saga ARP only | −14,100 / −14,088 | +1,000 / +996 | −752 / −740 | +1,004 / +1,000 |
| Both actual joined | **−17,020 / −17,012** | **+3,920 / +3,920** | **−3,664 / −3,656** | **+3,916 / +3,916** |

Actual POOL margin stays +512 after 8,192 reserve in both models on all maps. RAM text 2,748 B, core 1 text 420 B, XDATA 7,775 B unchanged. Guard map uses precisely the previous palette-safe single guarded HELP companion selection; Saga adds 12 B there via active ARP page descriptor size, 13,710 B total vs 13,698 B. This is strictly a map-of-failed-link measurement for new arms. No final guarded ELF/asset seal exists due to app-region overflow; neither a lab bank split nor a shipping layout change is included.

The largest structural joined text deltas on Elite are engine_render + audio ISR aggregate +702 B (LTO changes prior inlining; spending is from actual combined text, not the new standalone leaf's full 8,258 B), describe_pages +574, S1Sequencer_process +352, draw_param_widget +234, original patch read +184, undo +136, and arp_params +130. Inspect symbol-growth.log for individual and joint maps, normalized LTO-instantiation suffixes. These placements also rule out inference of timing from functional host test.

Joined host proof: Saga 2,176 checks against authenticated 640 division words, both owners, original rates/order, exact saved project, stock SPEED and isolation; S1 317 mixed checks and 194 oscillator edit checks. Host old compact reader 24 checks handles unmarked mixed objects and rejects new staged S1 markers without overwriting legacy data. S1-only vs both Synth One ARP/SEQ active PCM matches all 49,152 stereo frames, hash only. Focused regressions: 11,778 sync, 2,074 PWM, 40 S1 FILTER LINK, 126 ARP and 66 held-pad keyboard. Same existing nine controls and exact engine-qualified bit22 handling. The old-reader proof concerns the compact reader and S1 seam's actual records, not arbitrary new Saga-state backward transparency. Test files require the two respective Work packets, not the production-source patch.

Reproduce exact normal and guarded maps with existing pinned normal-link-only and HELP link helpers from sibling compact-fit, starting at optional owner branch. Measure separate S1-only and Saga-only patches and then combined patch in separate own checkouts. Whole map/digest compare and no original data export. Detailed actual counts saved in map-summary.log. No emulator, package, public source refs, hardware or production layout changes.
