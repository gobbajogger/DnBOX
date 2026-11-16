# Integration native memory evidence

## Exact candidate 13e0b087839c9606bab98680cabf3a2d74f74757

Independent test-and-merge native build and ELF inspection:

| Model | App-code free | Data+BSS used/capacity | Pool used/capacity | Pool free minus8192 floor |
|---|---:|---:|---:|---:|
| Elite | 13464B | 96792/98304B | 335036/344064B | 836B |
| Pro | 13472B | 96792/98304B | 335036/344064B | 836B |

Both native builds and budget subcheck PASS. Ordinary RAM free1512B.
`mst` remains664B and4-byte aligned; native symbol moved from pool address
0x01c47b78 to BSS address0x01c1c204. Pool shrank664B and BSS grew664B.
Startup zeroes both regions; `master_init` initializes the same full object.
No type, lifetime, DSP arithmetic, buffer length, control, or audio format changed.
Independent host master replay PASS. This is not full Local/visual/combined-lane qualification.

## Future combined owner constraints (not admitted)

Native Synth One initial owner10532B versus existing per-part DSP union3720B
would add13624B for two parts. Sound's separately verified serialized-held-state
candidate reduces its owner to8996B; actual integration is still outstanding.

Existing Chicago FX owns4252B per part separately. Source lifecycle audit:
- `original_presets.inc` restore and boundary paths retire before runtime init.
- `original_runtime_retire` calls Chicago backend retirement.
- `original_chicago_backend_retire` refuses an open block, discards the cold
  owner, and clears active/old/workspace/events/ready before successful return.
- Compact FX discard retires selection/state/queue.

This supports investigating per-part overlay of the mutually exclusive DSP+FX
owner lifetime (7972B), not a proven safe overlay. Returning to Chicago must
reinitialize its compact owner rather than rely on global once-only initialization;
never move/copy a live callback context or overwrite bytes after failed retirement.
Existing tail/adoption and event tests must pass before any memory credit.

Flash is a separate blocker: exact five authentic BREAK zones total109274B;
current default firmware embeds no bank. Synth One's separately measured
unlinked objects consume33406B executable +3230B readonly, plus165292B compact
wave pack, excluding additional integration data. No protected flash-map change,
user-slot substitution, lossy table change, polyphony reduction or false fit claim
is authorized by these measurements.
