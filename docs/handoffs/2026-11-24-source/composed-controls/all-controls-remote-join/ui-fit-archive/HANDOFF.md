# Non-overlapping archive fit reductions
Exact source-only archived 04,08,09 and 10,11,12 replayed with patch --fuzz=0 atop current ui-fit-outline source. archive-fit.patch is relative to that parent. No caps, feature disables, diagnostic layouts, test/device or package actions.

Fresh isolated paired guarded PI32 maps:
- UI outline baseline 27,932/27,924 B XIP overflow Elite/Pro.
- 04+08+09 = 27,484/27,476 B, save 448 B per board.
- those + 10+11+12 = 27,372/27,364 B, save 560 B per board over UI outline, or 8,164 B from pre-UI guard baseline.
- Reanchored 06 exact same side-effect display pairs regressed by 32 B on both; rejected and source reverted.
- 05 duplicated table setter; obsolete hunks. 13 existing detent logic differs from current UI and cannot be blindly imported; excluded, no hoped savings counted.
All six accepted archived changes are untested for runtime on this combined candidate. Same run.sh and inputs as baseline.
