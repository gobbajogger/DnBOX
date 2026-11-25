# Live BPM retiming on the guarded native SuperJ6 delay

This adds an emulator-only test to the already recorded centered-delay
five-note comparator. It checks that a legitimate Remote tempo change
reaches the **audible synced delay**, not just the DNR3 tempo status.
Use the same exact diagnostic firmware and disposable typed Init NOR
fixture from the wet comparator. The package and hardware are never used
as targets for writes. No browser or heavy-scene release qualification.

Send 120 BPM via DNR3 USB and independently read STATE (internal clock
throughout). Play one 40 ms note at 2.600 s, then set 125 BPM at 2.920 s
while the echo is active; independently read STATE again, and confirm
that the same settled SuperJ6 owner and all five delay controls remain
selected. An early repeated shot stops before the second tempo change;
another shot repeats the same schedule with wet output bypassed.

At detent 45 the internal-clock target is an eighth note. The clock
delays were separately checked via native DSP state: 11,025 frames at
120 BPM and 10,584 at 125 BPM. Compare time-marked 10 ms audio peak windows
to the bypassed control to verify the audible return and further repeats.
These windows do not provide a sample-accurate onset estimate. Inspect
the delay ring directly as a separate witness that history is warm.

For reproduction restore the verified centered guards and prior paired
run recipe. Generate the projections with the existing projector (no
automatic run), then prepare authored typed Init NOR for each model
using the previous authored-seed.py. Save the projection folder name in
the local projection file, run run.sh elite and run.sh pro serially,
then pass the tempo-native folder to summarize.py. The private worktree
paths in the scripts refer to the original qualification layout.
