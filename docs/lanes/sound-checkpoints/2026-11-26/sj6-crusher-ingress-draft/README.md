# Crusher canonical typed ingress, warm diagnostic only

**Do not adopt the control/address labels in this historical witness.**
This experiment proves standalone setter and receiver behavior, not which
installed AU parameter reaches the setter. The stronger
[source-mapped installed evidence](../sj6-crusher-source-mapped/README.md)
traces public #108 to the Amount/Decimation path, public #109 to a
metadata-only path, and internal #110 (not a registered public control)
to the Mix path. The earlier direct-call lab bound the laws to the wrong
addresses. Do not expose #109 as a decimation control on this evidence.

Pinned local firmware source hashes in pins.txt. Present on-unit Effect
page uses CRUSH (two positions 0/1), AMOUNT (integer 0..127), backed by
pref_set_impl → pref_set_typed_value → typed_1 → sj6_crush_choose. SMC and
panel pass integer detents through the descriptor limits. DNR3 for this exact
firmware maps SJ6 #107 and #108 to the same typed controls. rd_synth_write
decodes an integer with rd_g(q+6,2), validates descriptor max, expected old
value and owner generation, then calls ui_remote_synth; that invokes the
same pref_set_impl and returns the actual readback. #109 is absent on-unit,
in remote_sj6_panel.inc/extra, and from the Remote client's routable catalog
(its glyph registry alone does not make it routable). No raw AU float traffic.

The only proven projected carrier is two existing bytes at
sound.v[T_MST][SJ6_CRUSH_ROW+owner][SJ6_CRUSH_BYTE + slot]. UI Undo validates
(<=1, <=127), marks the native-room mask then reapplies the existing room
and Crusher code. Restore and reset likewise use the old two-byte carrier.
The existing indexed choose discards equal values or blocked owner and calls
store only under engine_project_hold. Order matters: a snapshot of two
bytes cannot tell whether previously applied #107 reset #108 wet, nor
recover #109 or the graph. Undo must not fabricate a series of new physical
writes from the bytes; source-profile admission is revoked per owner instead.
No project format/version, UI string, or stored-word change.

candidate.patch observes one accepted step inside the actual store
transaction, converts only that input with unit_control(q), and offers it to
the tested physical converter only when an independent witness lease exists.
It never sends the result to the renderer or replaces sj6_crush_apply.
Conditional host witness only, not production. Undo revokes the selected
owner's lease under the original transaction before old DSP apply. Ordinary
unknown/legacy goes through original DSP. Non-contract #109 stays unsupported
on this carrier; adding original rate requires explicit persistent typed
field, previous effective-state restoration or revocation, and authentic
graph + renderer qualification. Existing #108 LFO to legacy cadence is
separate, dynamic, not a 7-bit edit conversion and must not silently adopt
the new wet route.

test.py asserts the exact Remote and on-unit source mapping and compiles the
real extracted indexed choose + Crusher getters/apply/store + Undo entry
with a bounded host transaction fixture and real converter. Tests pass
accepted/blocked/no-op/oversized, exact hold, legacy frozen history, verified
shadow updates, per-owner Undo, and no stale proof re-arm. Not browser E2E,
full UI, native replay, native fit or signed runtime graph. Keep app off.

Replay: python3 test.py PATH/firmware/src/app. It uses the separately sealed
sj6-crusher-converter directory next to this one.
