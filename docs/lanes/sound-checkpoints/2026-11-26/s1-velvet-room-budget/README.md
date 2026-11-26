# Velvet-noise reverb feasibility, no host replacement admitted
Primary 2021 IVN (Välimäki/Prawda): DOI 10.1109/TASLP.2021.3060165,
https://acris.aalto.fi/ws/portalfiles/portal/94807831/Late_Reverberation_Synthesis_Using_Interleaved_Velvet_Noise_Sequences.pdf
Figures 4–6 explain SFIR/comb history sharing *within each* independent
filtered branch, not across differently filtered branches. Four illustrative
EVN lengths 7760/8080/8240/8560, sum 32640 live cells before smearing,
additional small routing delays or filter state. Branched mono-fed output
stereo is 32,640B/owner at only 8 bits or16,320 B at 4 bits raw ideal packing.
Distinct stereo input branches run conservatively as two independent fed
banks: 65,280B stereo/owner at 8 bits. An exact single-input stereo reduction
would discard genuine source side ingress (measured anti/common ~0dB).
At /4 rate and 4 bits the stereo core is 8160B before at least dozens of
cursors/indices, loop state, mono-to-stereo mixing/smearing, interpolation,
codec state and early taps. 11.025k total clock -> 5.5125k Nyquist. Shorten
below this to fit by sacrificing the paper's late periodicity properties
or further cutting the bandwidth; neither preserves source identity.

408 minimum nonzero taps in published configuration and 802 operations
per sample (paper Table II, one conventional output), 410,624 operations
per 512-frame callback before a second owner or distinct stereo input or
reconstruction. Representative packed offsets+signs 816 B of immutable
tap data, excluding decode code; storing four sets of 10 SOS + one shelf
coefficients a further ~848B when precomputed, perhaps mutable if controls
are live. Estimate neither physical MAC timing nor code linked size.
Even a tiny ring does not follow from sparsity of the response.

Distinct 2020 VFDN (Fagerström/Alary/Schlecht/Välimäki):
https://dafx2020.mdw.ac.at/proceedings/papers/DAFx2020_paper_23.pdf
This method uses sparse short sequence diffusion at ordinary FDN
inputs/outputs. It can improve early echo density independently of whether
an IVN comb can fit; not a re-labeling of IVN. Their specific 16-line core
still sums 48,544 samples, 24,272 B with ideal 4-bit full-rate packing.
VN15: 10ms/15 taps per in/out line. /4-rate 4-bit core about 6072B plus
~999B independent extra half-byte in/out histories (favorable simple
single stereo-fed base ring with per-output extensions). Per-line filter
and matrix state, delay codec and clock reconstruction not accounted.
Coefficients 32×15 packed signed/tap words of 2 bytes ~960B unless regenerated;
one independent coupled 16-line tank can map stereo ingress/egress, but
no evidence of preserving original stereo transfer or feedback decay.
Published 1,296/1,776 FLOP/sample vs 848 for same core without filters, or
2,720 for the compared 32-line reference. Hypothetical four-line add-on
using the prior 3,748B test core can reuse existing early inputs but adds
up to ~888B for 4×111 BF16 output filter history, plus mapping and 120
sparse signed additions per wet tick. 4-line VFDN under this budget is
not validated by the paper or an exact source sound model. It changes
early density; does not restore RevSC's moving eight feedback poles,
time-varying lane phase, stereo side lifetime or gain. Previous Organ
late stereo side/level fail unchanged in the absence of a causal fix.

Exact local stand-alone RevSC source 44.1k impulse witness 16s
(from preexisting tested source harness). Signed stereo ingress pulse
pairs (L/R separate, equivalent by linearity at fixed fb) compare output
with equal and opposite input sum. No audio captures or installed apps.
Integrated-band E60 from .1s conditional late energy, not RT60 curve fit.
Unbounded reverbs can have quiet activity beyond cutoff; do not use
finite-window E60 as exact universal impulse length. Mono-only removes
all input-side response despite genuine late signal on both channels.

Conclusion: neither literal transfer design is admitted for contemplated
4–8 KiB owner with very tight callback headroom and exact late stereo.
No new synthesis/Organ/Pad candidate rendered, no gate adjustments,
firmware, user hardware or binary included.

Upstream RevSC source is omitted here. The exact Library reproducer has SHA256 19561909a403bcbcd0ace525d7d127032b839b21d92914be24a7f3eaf4f61b12.
