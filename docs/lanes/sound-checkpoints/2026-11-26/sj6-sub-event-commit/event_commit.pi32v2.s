	.text
	.file	"/workspace/scratch/3a7051e761a0/recovered-sound/sj6-sub-handoff/minimal-commit/event_commit.c"
	.globl	sj6_event_offer
	.p2align	1
	.type	sj6_event_offer,@function
sj6_event_offer:                        ; @sj6_event_offer
; BB#0:
	[--sp] = {rets, r15-r4}
	r9 = r2
	r14 = r3
	r7 = r0
	r8 = r1
	r3 = 1072693248
	r2 = 0
	r4 = 0
	r1_r0 = r9_r8
	call __gtdf2
	r5 = 1
	r6 = 1
	ifs (r0 <= 0) {
		r6 = 0
	}
	r3_r2 = 0
	r1_r0 = r9_r8
	call __gtdf2
	r1 = 1
	ifs (r0 >= 1) {
		r1 = 0
	}
	r0 = r6 | r1
	if (r14 != 0) {
		r5 = 0
	}
	r15 = [sp+52]
	if (r15 > 127) goto .LBB0_9
; BB#1:
	if (r14 > 128) goto .LBB0_9
; BB#2:
	r0 |= r5
	if (r0 != 0) goto .LBB0_9
; BB#3:
	r11 = r9 ^ 0x80000000
	r10 = r8
	r0 = r14
	call __floatunsidf
	r3_r2 = r1_r0
	r0 = r8
	r1 = r11
	call __divdf3
	r11_r10 = r1_r0
	r3_r2 = 0
	call __ltdf2
	ifs (r0 > -1) goto .LBB0_9
; BB#4:
	r4 = r14 + 1
	r5 = 0
	r0 = 1
	r13_r12 = r9_r8
.LBB0_5:                                ; =>This Inner Loop Header: Depth=1
	r6 = r0
	r1_r0 = r11_r10
	r3_r2 = r13_r12
	call __adddf3
	r13_r12 = r1_r0
	r3_r2 = 0
	call __gtdf2
	r5 += 1
	ifs (r0 < 1) goto .LBB0_7
; BB#6:                                 ;   in Loop: Header=BB0_5 Depth=1
	r0 = r6 + 1
	if (r6 < r4) goto .LBB0_5
.LBB0_7:
	r3_r2 = 0
	r4 = 0
	r1_r0 = r13_r12
	call __gtdf2
	ifs (r0 > 0) goto .LBB0_9
; BB#8:
	d[r7+0] = r9_r8
	h[r7+8] = r14
	h[r7+12] = r5
	h[r7+10] = r5
	b[r7+14] = r15
	r4 = 1
.LBB0_9:
	r0 = r4
	{pc, r15-r4} = [sp++]
.Lfunc_end0:
	.size	sj6_event_offer, .Lfunc_end0-sj6_event_offer

	.globl	sj6_event_retarget
	.p2align	1
	.type	sj6_event_retarget,@function
sj6_event_retarget:                     ; @sj6_event_retarget
; BB#0:
	[--sp] = {rets, r11-r4}
	sp += -4
	r4 = r0
	r8 = r1  #
	r5 = h[r4+12] (u)
	if (r5 == 0) goto .LBB1_5
; BB#1:
	r7_r6 = d[r4+0]
	r0 = 0
	r1 = 0x80000000
	r3_r2 = r7_r6
	call __subdf3
	r11_r10 = r1_r0
	r9 = h[r4+8] (u)
	r0 = r9
	call __floatunsidf
	r3_r2 = r1_r0
	r1_r0 = r11_r10
	call __divdf3
	r11_r10 = r1_r0  #
	r2 = h[r4+10] (u)
	r5 = r2 - r5
	goto .LBB1_3
.LBB1_2:                                ;   in Loop: Header=BB1_3 Depth=1
	r1_r0 = r11_r10
	r3_r2 = r7_r6
	call __adddf3
	r7_r6 = r1_r0
	r5 += -1
.LBB1_3:                                ; =>This Inner Loop Header: Depth=1
	if (r5 != 0) goto .LBB1_2
; BB#4:
	[sp] = r8
	r0 = r4
	r1 = r6
	r2 = r7
	r3 = r9
	call sj6_event_offer
	goto .LBB1_6
.LBB1_5:
	r0 = 0
.LBB1_6:
	sp += 4
	{pc, r11-r4} = [sp++]
.Lfunc_end1:
	.size	sj6_event_retarget, .Lfunc_end1-sj6_event_retarget

	.globl	sj6_event_prep
	.p2align	1
	.type	sj6_event_prep,@function
sj6_event_prep:                         ; @sj6_event_prep
; BB#0:
	r1 = r0
	r2 = h[r1+12] (u)
	r0 = -1
	if (r2 == 0) goto .LBB2_3
; BB#1:
	r2 += -1
	r3 = r2.l (u)  #
	h[r1+12] = r2
	if (r3 != 0) goto .LBB2_3
; BB#2:
	r0 = b[r1+14] (u)
.LBB2_3:
	rts
.Lfunc_end2:
	.size	sj6_event_prep, .Lfunc_end2-sj6_event_prep

	.globl	sj6_event_cancel
	.p2align	1
	.type	sj6_event_cancel,@function
sj6_event_cancel:                       ; @sj6_event_cancel
; BB#0:
	r1 = 0
	h[r0+12] = r1
	rts
.Lfunc_end3:
	.size	sj6_event_cancel, .Lfunc_end3-sj6_event_cancel


