	.text
	.file	"target-plain.c"
	.globl	normal_span_f32
	.p2align	1
	.type	normal_span_f32,@function
normal_span_f32:                        ; @normal_span_f32
; BB#0:
	[--sp] = {rets, r11-r4}
	if (r3 == 0) goto .LBB0_2
.LBB0_1:                                ; =>This Inner Loop Header: Depth=1
	r8 = [r0+12]
	r9 = [r1+16]
	r10 = [r1+0]
	{r11, r4} = [r0+]
	r5 = [r2+0] 
	r6 = [r0+8] 
	r7 = [r1+12] 
	[r0+4] = r4
	[r0+0] = r5
	r6 = r7 * r6 (f)  #
	[r0+12] = r6
	r5 = r5 + r11 (f)
	r4 = r4 + r4 (f)
	r4 = r5 + r4 (f)
	r4 = r10 * r4 (f)
	r4 = r4 - r6 (f)
	r5 = r9 * r8 (f)
	r4 = r4 - r5 (f)
	[r0+8] = r4
	[r2++=4] = r4
	if (--r3 != 0) goto .LBB0_1
.LBB0_2:
	{pc, r11-r4} = [sp++]
.Lfunc_end0:
	.size	normal_span_f32, .Lfunc_end0-normal_span_f32


