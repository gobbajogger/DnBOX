Test on the exact H16 Widen donor: apply sine.patch then ladder.patch to s1u.c with zero fuzz. Both source leaves retain the old operation order, including the ladder's first add with literal positive zero. Host oracle covers 16,712,704 exact sine outputs and 1,048,576 exact ladder outputs and all five state values. Native link: 24 / 36 B free XIP and unchanged 564 B above the pool reserve. No installable package was generated.

The unprofiled strict native replay uses the exact 922-event tape, same six project sectors, same active original-width target and sealed synthetic project as the H16 reference. Every run had zero skips and zero virtual NOR programs or erases. Sine + ladder lowered peak but worsened underruns on both models:
- Elite: 15,558 µs / 39 → 15,224 µs / 50.
- Pro: 15,793 µs / 53 → 14,196 µs / 85.

Owner polls and timer placement vary naturally between separate images; this is not bitwise equivalence of their overloaded whole mixer timelines. The numerical host leaves are exact. The combined change is rejected as a load-gate improvement. Continue with a separately measured compressor change and scheduling investigation instead of accumulating changes based on peak alone.
