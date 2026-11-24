# Combined Synth One LFO and SuperJ6 reference tuning

One source checkpoint, not a package. Start at the combined authored-vibrato
and Saga source. First apply s1-lfo-full-control.patch and verify the 313
product source hashes against the pinned LFO packet. Apply the A-reference
source and host-test patches after that. The A-REF pre/post checksum sets here
pin the exact order and shared source after the LFO patch, not the A REF
worker's earlier donor.

The LFO controls on Synth One include physical Rate and Shape for both
oscillators' LFOs, and Sync to Arp. A temporary note lock restores the rates,
auto-pan and delay atomically when it ends. Existing qualified host evidence
verified unchanged Init and 694 programs. Older project readers refuse new
marked records without overwriting them. SuperJ6 A REF is original ID 206,
200–600 Hz with exact default 440 Hz; authored off-grid values are preserved.
It does not claim per-layer coarse or fine tuning.

The actual one candidate after both patches passed 382 cross-engine
ownership and LFO tests and the 7,594-check A REF test with both owners,
exact saved and reload audio, and no audible changes on the unaffected peer.
The shape/rate and sync rollback replay from the pinned LFO stage remains
separate evidence as documented. No native timing, full 64, XIP, installer
or website release is claimed. No private source bank in Git.
