# Motion runtime storage: one byte per lane

The old live ramp uses two bytes: active flag and starting-step parity. The
starting parity is observed only while the ramp is running. The candidate
stores 0 (off), 1 (active/even), or 2 (active/odd) in the existing ramp byte.
A disable remains a single zero store. No changes to the saved 72-byte lane,
on-wire data, note flags, lane capacity, recording and hold mailboxes, or project
backup/undo format. The candidate does not conflate unrelated state owners.

Pinned source: 47014285, candidate SHA256
`125a0d98dc7ac8aba7e1b47a7ed2d0eeb862983697ff22dd08c916b91538e44a`.

Apply `candidate.diff` in an isolated checkout; compile the four existing
standalone motion tests against `firmware/src/seq/motion.c`, then repeat with
`differential.c` using `cc -std=c11 -O2 -Wall -Wextra -Werror` and include
`firmware/src/seq`. Compare stdout to the unmodified source build.

Verified: motion, clear-pending, pattern-handoff and pattern-pending tests
produce identical passing stdout before and after. In the differential,
180,000 actions include slot reuse, record and hold mailbox traffic, alternation
of restore modes, odd/even playback, transport release, and raw lane edits; both
run 218,400 identical hashed writes/restores and produce `2160a4f6d28dff59`.
The 160 records are 11 -> 10 bytes; host motion object 1,872 -> 1,712 bytes.
No bytes credited in the live firmware: compare both native maps, ensure the
relocated memory is genuinely available to the intended owner, and qualify
worst-case IRQ timing. Full native build remains blocked by runner ENOSPC.

## Adjacent lanes, checked independently

- UAC capture and playback can be at full occupancy simultaneously. The USB
  fixture observed 1,535 + 1,024 pending stereo frames, already packed PCM16.
- I2S DMA uses both contiguous halves of the 8,192-byte audio buffer; silence
  reuses these, so there is no redundant shadow to remove.
- Core 1 stack is 4,352 user + 256 exception bytes. Recorded watermarks are
  scene bounds only; exception storage cannot overlap suspended user frames.
- A 28-bit command ring would save 512 bytes with 410 bytes isolated added
  flash. Rejected for current raw behavior: an out-of-range bass voice ID that
  was ignored can alias a real bass voice. A typed sender redesign is a
  separate option, not credited.
- The authentic 8-line Soundpipe room replay matches full-rate gold; /2 and
  /4 change its signed stereo return substantially and don't fit its present
  reverb arena even at one byte/history sample. No borrowing from a coactive
  effect, no lower-rate rollout.
