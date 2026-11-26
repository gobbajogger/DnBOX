	.text
	.file	"sj6-compact-halfband/target.c"
	.p2align	1
	.type	pole,@function
pole:                                   ; @pole
; BB#0:
	[--sp] = {rets, r4}
	r4 = r0 * r1 (f)  #
	r3 = [r2+0] 
	r3 = r4 + r3 (f)
	r1 = r3 * r1 (f)
	r0 = r0 - r1 (f)
	r0 = r3  #
	[r2+0] = r0
	{pc, r4} = [sp++]
.Lfunc_end0:
	.size	pole, .Lfunc_end0-pole

	.p2align	1
	.type	halfband,@function
halfband:                               ; @halfband
; BB#0:
	[--sp] = {rets, r6-r4}
	r4 = r2
	r5 = r0
	r2 = 1034129666
	r0 = r1
	r1 = r2
	r2 = r4
	call pole
	r6 = r0
	r2 = r4 + 4
	r1 = 1049711124
	r0 = r5
	call pole
	r5 = r0
	r2 = r4 + 8
	r1 = 1057725516
	r0 = r6
	call pole
	r6 = r0
	r2 = r4 + 12
	r1 = 1062575109
	r0 = r5
	call pole
	r0 = r6 + r0 (f)
	r1 = 0x3F000000
	r0 = r0 * r1 (f)
	{pc, r6-r4} = [sp++]
.Lfunc_end1:
	.size	halfband, .Lfunc_end1-halfband

	.globl	measured
	.p2align	1
	.type	measured,@function
measured:                               ; @measured
; BB#0:
	[--sp] = {rets, r6-r4}
	r4 = r1
	r5 = r0  #
	r1 = [r4+4] 
	r2 = [r4+0] 
	r6 = r5 + 4
	r0 = r2
	r2 = r6
	call halfband
	r2 = [r4+8] 
	r4 = r0  #
	r1 = [r4+12] 
	r0 = r2
	r2 = r6
	call halfband
	r1 = r0
	r2 = r5 + 20
	r0 = r4
	{rets, r6-r4} = [sp++]
	goto halfband
.Lfunc_end2:
	.size	measured, .Lfunc_end2-measured

	.globl	footprint
	.p2align	1
	.type	footprint,@function
footprint:                              ; @footprint
; BB#0:
	r0 = 36
	rts
.Lfunc_end3:
	.size	footprint, .Lfunc_end3-footprint


