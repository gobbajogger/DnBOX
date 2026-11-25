# Stop forcing unreferenced bank entry points in the Init-only native profile

On exact source-wheel-native-scale ledger 8cf74b7a (unmodified SDK,
firmware, geometry and locked inputs), both stock linkers forcibly kept
five exported bench/legacy entry points. The app source contains their
declarations and definitions but no call sites. The real physical Filter
Link goes through its current panel setter, not the old standalone bench
function. No user-facing controls, gate, engine, array, loader or flash
map is removed.

Apply prune.patch to tools/build.py after the exact wheel-native-scale
source. The historical explicit banked profile retains its names. The
Init-only condition omits only five forced symbols; ordinary reachable code
is not altered. Forced-list instrumented stock linker and ordinary
entrypoint replay of the patched source both show 1,296 B reclaimed per
board (effective XIP overflow Elite 32,544 to 31,248, Pro 32,532 to 31,236).
No missing symbols. Both stock links are still red. This is not native
audio/timing, installation, release, or full-control fitness. Newer
host-only controls were not in this link.
