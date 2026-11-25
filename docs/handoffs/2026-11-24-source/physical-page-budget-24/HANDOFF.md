# One more physical four-knob page without extra page-table storage

Exact implemented wheel-native-scale source, manifest 8cf74b7a… .
Group IDs occupy one nibble; the two bounded page/overflow counts are
each one byte. The cached page table moves from 424 to 422 bytes. The
existing 276-byte expanded list is unchanged. The same actual page
emitter serves physical UI and SMC queries. No page contents, knobs,
clock, original ADSR LINK, storage format, flash map or DSP changed.

Whole-app host before/after comparisons on legal view/part pairs yielded
710 identical descriptor, SMC, hidden-list and renderer-hash lines.
V_GEN is excluded for non-bass parts because physical entry is only
allowed on bass and the production part picker leaves GEN otherwise.
The new slot check additionally exercises a 24th real descriptor across
both cached and queried paths, verifies every packed group and rejects a
25th. UBSan checks 1,330 before and 1,357 after. The existing list cap is
unchanged; don't assume the list can contain 96 arbitrarily full rows.
No native fit/performance or released screen claim.

Replay: sh replay.sh /exact/source-wheel-native-scale
