# Authored-wet dual-room is not release eligible
Strict original 922-event scene, offset 0, both models, exactly sealed
ELFs, settled Synth One A + active arp and Chicago B. No skips, writes,
unsafe guard state or invalid RAM overlays. Both cold and authored
profiles retain independent Delay history partition 63. All rows fail
the timing budget; raw late equals xruns here.

| Profile/model | Cold-Undo RV4 baseline max µs / XRUN | Exact authored-wet candidate max µs / XRUN |
| --- | ---: | ---: |
| Dry Init Elite | 15,512 / 138 | 15,387 / 25 |
| Dry Init Pro | 15,368 / 43 | 15,514 / 137 |
| Authored wet Elite | 15,513 / 138 | 18,949 / 134 |
| Authored wet Pro | 15,352 / 31 | 15,710 / 130 |

In actual target-ABI RAM the candidate selects four-line room at dry
RVMIX=0 with stable room feedback 0.5, eight-line room sentinel -2 at
RVMIX=.7559055, and matching allocated per-owner arena in all cases.
So the original candidate's inadvertent wet-zero eight-line choice is
fixed, not the performance limit. On wet Elite, final Core1 was on and
alive with no give-up or bad mailbox, and 1,026/1,026 dispatches done;
no cause is inferred for its timing regression from a single offset.
No applied package, device operation, or production routing change.
Synthetic reverb-active Init is intentionally not the authenticated
Organ preset; authored Organ and five other controls are host-only in
the six-match audio fixture (see FIT.md for matched captures).
