	.text
	.file	"sj6-compact-halfband/target.c"
	.p2align	1
	.type	encode,@function
encode:                                 ; @encode
; BB#0:
	r1 = 0x46800000
	r1 = r0 * r1 (f)
	r2 = 1191181824
	r0 = 32767
	iff (r1 >= r2) goto .LBB0_3
; BB#1:
	r2 = 0xC7000000
	r0 = -32768
	iff (r1 <= r2) goto .LBB0_3
; BB#2:
	r0 = 0
	iff (r1 u>= r0) {
		r0 = r0 + 0x3F000000
	} else {
		r0 = r0 + 0xBF000000
	}
	r0 = r1 + r0 (f)
	r0 = ftoi(r0) (trunc)
.LBB0_3:
	rts
.Lfunc_end0:
	.size	encode, .Lfunc_end0-encode

	.p2align	1
	.type	qp,@function
qp:                                     ; @qp
; BB#0:
	[--sp] = {rets, r4}
	r3 = r0
	r0 = h[r2+0] (s)
	r4 = r1 * r3
	r4 = r4 + 0x4000
	r4 = r4 >>> 15
	r0 += r4
	if (r3 == 0) goto .LBB1_2
; BB#1:
	r1 *= r0
	r1 = r1 + 0x4000
	r1 = r1 >>> 15
	r1 = r3 - r1
	goto .LBB1_3
.LBB1_2:
	r1 = 0x0 - r1
	r1 *= r0
	r3 = r1 >>> 31
	r4 = 32767
	r3 &= r4
	r1 += r3
	r1 = r1 >>> 15
.LBB1_3:
	r3 = 32767
	ifs (r1 < 32768) {
		r3 = -32768
		r3 = smax(r1, r3)
	}
	h[r2+0] = r3
	{pc, r4} = [sp++]
.Lfunc_end1:
	.size	qp, .Lfunc_end1-qp

	.p2align	1
	.type	qhalf,@function
qhalf:                                  ; @qhalf
; BB#0:
	[--sp] = {rets, r6-r4}
	r4 = r2
	r2 = r1
	r5 = r0
	r1 = 2617
	r0 = r2
	r2 = r4
	call qp
	r6 = r0
	r2 = r4 + 2
	r1 = 9301
	r0 = r5
	call qp
	r5 = r0
	r2 = r4 + 4
	r1 = 17870
	r0 = r6
	call qp
	r6 = r0
	r2 = r4 + 6
	r1 = 27342
	r0 = r5
	call qp
	r0 += r6
	r0 += 1
	r0 = r0 >>> 1
	{pc, r6-r4} = [sp++]
.Lfunc_end2:
	.size	qhalf, .Lfunc_end2-qhalf

	.globl	measured
	.p2align	1
	.type	measured,@function
measured:                               ; @measured
; BB#0:
	[--sp] = {rets, r7-r4}
	r5_r4 = r1_r0
	r1 = [r5+0] 
	r0 = r1
	call encode
	r3 = r0  #
	r1 = [r5+4] 
	r0 = r1
	call encode
	r1 = r0
	r6 = r4 + 4
	r0 = r3
	r2 = r6
	call qhalf
	r7 = r0  #
	r1 = [r5+8] 
	r0 = r1
	call encode
	r3 = r0  #
	r1 = [r5+12] 
	r0 = r1
	call encode
	r1 = r0
	r0 = r3
	r2 = r6
	call qhalf
	r1 = r0
	r2 = r4 + 12
	r0 = r7
	call qhalf
	r0 = itof(r0)
	r1 = 0x38800000
	r0 = r0 * r1 (f)
	{pc, r7-r4} = [sp++]
.Lfunc_end3:
	.size	measured, .Lfunc_end3-measured

	.globl	footprint
	.p2align	1
	.type	footprint,@function
footprint:                              ; @footprint
; BB#0:
	r0 = 20
	rts
.Lfunc_end4:
	.size	footprint, .Lfunc_end4-footprint


