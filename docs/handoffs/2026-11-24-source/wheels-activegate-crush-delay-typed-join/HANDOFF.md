# Same-tree SuperJ6 Delay controls and ID118 sync

Apply delay-sync-join.patch to the exact active GATE + CRUSH checkpoint;
verify the supplied pre/post hashes. Keeps b20 and all prior authored controls
and assigns only b16–19 as previously approved. App history uses the existing
single owner hold path, not a second nested hold. Both source and fixtures are
host-only. The scoped external float history is 11026 samples per channel per
owner; outside the allowed time at low tempo external/active history persists
dry without rewriting authored mode/time. This is not native fit, original AU
whole-engine or release evidence.

Replay: bash replay.sh [exact-source-mod-wheels-directory]. Runs same-tree
both owner orders in one real mixed project, two-Juno short ring/clock refusal
and history/recall, then restores exact prior active GATE+CRUSH sources and
checks the new mixed writer media is forbidden before writes. Clearing only
b16–19 admits the otherwise identical stage.
