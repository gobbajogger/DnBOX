	.text
	.file	"target.c"
	.globl	typed_sine_profile_prepare
	.p2align	1
	.type	typed_sine_profile_prepare,@function
typed_sine_profile_prepare:             ; @typed_sine_profile_prepare
; BB#0:
	[--sp] = {rets, r8-r4}
	if (r3 == 0) goto .LBB0_6
; BB#1:
	r3 = 0
	r4 = 0
	r5 = 1062131991
	r8 = 1067347951
.LBB0_2:                                ; =>This Inner Loop Header: Depth=1
	r6 = b[r1++=1] (u)
	if (r6 != 2) goto .LBB0_5
; BB#3:                                 ;   in Loop: Header=BB0_2 Depth=1
	r6 = [r2+r3]
	iff (r6 u!= r4) goto .LBB0_5
; BB#4:                                 ;   in Loop: Header=BB0_2 Depth=1
	r7 = r0
	r6 = [++r7=r3]
	iff (r6 <= r5) {
		r6 = r6 * r8 (f)
		[r7+0] = r6
	}
.LBB0_5:                                ;   in Loop: Header=BB0_2 Depth=1
	r3 += 4
	if (r3 != 8) goto .LBB0_2
.LBB0_6:
	{pc, r8-r4} = [sp++]
.Lfunc_end0:
	.size	typed_sine_profile_prepare, .Lfunc_end0-typed_sine_profile_prepare

	.globl	typed_sine_hot_receiver
	.p2align	1
	.type	typed_sine_hot_receiver,@function
typed_sine_hot_receiver:                ; @typed_sine_hot_receiver
; BB#0:
	r2 = r0
	r0 = r2 * r1 (f)
	r3 = 0x3F800000
	iff (r1 == r3) {
		r0 = r2
	}
	rts
.Lfunc_end1:
	.size	typed_sine_hot_receiver, .Lfunc_end1-typed_sine_hot_receiver


