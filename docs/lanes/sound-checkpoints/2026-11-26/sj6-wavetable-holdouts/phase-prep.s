	.text
	.file	"phase-prep.c"
	.globl	sj6_phase_prep
	.p2align	1
	.type	sj6_phase_prep,@function
sj6_phase_prep:                         ; @sj6_phase_prep
; BB#0:
	r1 = 1072114849
	r1 = r0 * r1 (f)
	r1 = r1 * r0 (f)
	r2 = 1064863198
	r1 = r1 + r2 (f)
	r0 = r1 * r0 (f)
	rts
.Lfunc_end0:
	.size	sj6_phase_prep, .Lfunc_end0-sj6_phase_prep


