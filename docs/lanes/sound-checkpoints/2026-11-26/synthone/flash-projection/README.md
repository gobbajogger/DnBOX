# Current matched Synth One: PI32v2 code-size probe
Input: DSP source generated for the 906-program matched host run
(c58f79998f360f42fa51535d302e3260e5cc1748), not an older firmware effect approximation.
Compare exact flags off/on on the same source with Pi32v2 r3 FPU, section GC,
same public roots, and no sound or control cuts. The standalone runtime arm also
links the target r3 libm/libc/compiler-rt archives. These are relocatable app
projections, not sealed board images. See unresolved imports in result.json.
The host-only trace functions may still be referenced by this measurement.
Neither differential is a certified upper/lower bound on the integrated build.

Matched delay/reverb/stereo subgraph: 8,416 B before external dependencies.
Total instrument DSP difference: +4,662 B (direct allocations only). Adding
runtime objects to both arms: +14,021 B. To final-link this exact graph, port the
allocation/diagnostic boundaries and account for native math-code sharing;
current reference-delay state cannot be allocated on-device. This checkpoint
does not claim a native A/B, final flash/ram margin, timing pass or release.
