	.text
	.file	"target.c"
	.globl	normal_span_f32
	.p2align	1
	.type	normal_span_f32,@function
normal_span_f32:                        ; @normal_span_f32
; BB#0:
	[--sp] = {rets, r13-r4}
	r4 = r3
	r5 = r2
	r11 = r1
	r7 = r0
	if (r4 == 0) goto .LBB0_2
.LBB0_1:                                ; =>This Inner Loop Header: Depth=1
	r8 = [r7+12]
	r12 = [r11+16]
	r13 = [r11+12]
	{r9, r2} = [r7+]
	r10 = [r11+8]
	{r3, r0} = [r11+]
	r1 = [r5+0] 
	r6 = [r7+8] 
	[r7+] = {r2, r1}
	r2 = r3 * r2 (f)  #
	[r7+12] = r6
	call fmaf
	r2 = r0
	r0 = r10
	r1 = r9
	call fmaf
	r2 = r0
	r0 = r13 ^ 0x80000000
	r1 = r6
	call fmaf
	r2 = r0
	r0 = r12 ^ 0x80000000
	r1 = r8
	call fmaf
	[r7+8] = r0
	[r5++=4] = r0
	if (--r4 != 0) goto .LBB0_1
.LBB0_2:
	{pc, r13-r4} = [sp++]
.Lfunc_end0:
	.size	normal_span_f32, .Lfunc_end0-normal_span_f32


