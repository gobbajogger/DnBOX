# Synth One dense, seekable exact delay storage: rejected
Source is the four independent dated write histories, recovered from true
host Soundpipe exact delay taps. Prior source capture produced exact tap
matches at all 264,728 records for Pad/Hall/Deeper/Infinite Tension. The
current witness checks capture lengths and original two summed right taps
at load. Uses actual settled chronological physical windows (two 2T,
two T), never pitch/delay aligned or bandwidth-reduced. A bounded 4-cell
guard extends each required window for cubic history access.

Test 64/128 samples per record, independent per line or parallel exact
word transform on equal absolute sample epochs. For each record pick
the smallest of raw, independent raw DEFLATE with byte shuffle/predictor
or signed ZigZag Rice with explicit seeds. Reversible transforms include
word XOR/unsigned modulo arithmetic delta1/delta2, plus optional
cross-lane XOR (not mono or lossy folded short histories). Each chosen
record resets its coder, is directly indexable and decodes **word for
word**. No same preset seeding, linear signal approximations or loss of
F32 bits in the exact mode. The per-record choice and DEFLATE are
optimistic offline as to code and CPU, not a product implementation.

Actual byte costs: 4-byte per-record type/parameters/length, 4-byte index
offset, method seeds as payload, raw current open block plus 128 bytes
state. Oldest boundary block is fully live rather than falsely assuming
byte-level free overlay; data is independent and variable-length. No
allocator fragmentation, in-place recompression shadow buffer, peak
decode scratch or worst-rate hot loop assumed. Allowing these would
only raise the bound, and moving mutable input to flash is not durable
or real-time. Worst measured 128-frame held Pad cross record 1817 B plus
4-byte index, versus raw hard fallback 2052 B including header for
4 lanes. Worst independent lane lengths per scheme and all ages
recorded in result.jsonl. Four cubic heads can cross two block boundaries
each (worst eight independent decodes before caching); random access does
not mean a single memory-load in a predictor.

Smallest measured exact windows bytes: Pad 199,330, Hall 571,981,
Deeper 95,575, Infinite 1,368,368. For the single best fixed format in
each cue (128, grouped dated lanes), worst measured window is 200,378 /
588,863 / 101,470 / 1,581,155 B. Per-format selected values and worst
blocks are part of the numeric JSON, not an extrapolation to all future
notes. This does not say all reversible coding is impossible.

Comparative lossy baseline: first round original writes to IEEE binary16
(post-round arithmetic still lossless **only to those rounded words**;
not claimed equivalent to stock H16 edge handling or actual controller
parity). In best fixed format, max snapshot bytes 97,987/258,383/
21,021/718,234. A quiet Deeper example clears earlier generous 25,792 B
per owner across selected windows but does not clear even pooled 9,068 B;
no suitable per-preset exceptions. No F16 complete audio render or
audibility conclusion in this study.

4,534 B per owner is optimistic hypothesis replacing both known 4,252B
shared wet lines and using 564 B above protected 8192, not a new SRAM
grant. Even pooled architecture 9068B total is below all cases. No
native target build/integration attempted; actual current room/delay
behavior untouched. Reproduce by regenerating .state and .write with
the previous dated-residual reference harness, validating inputs.sha256,
and running OPENBLAS_NUM_THREADS=1 python sweep.py. Codec only consumes
existing state/captures, which are not present in this archive.
