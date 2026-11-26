	.text
	.file	"exact_fused.c"
	.p2align	1
	.type	shift,@function
shift:                                  ; @shift
; BB#0:
	[--sp] = {rets, r7-r4}
	r4 = r3 >> 5
	r3 = r3 & 0x1F
	r5 = 0x20 - r3
	goto .LBB0_7
.LBB0_1:                                ;   in Loop: Header=BB0_7 Depth=1
	if (r4 > 5) goto .LBB0_3
; BB#2:                                 ;   in Loop: Header=BB0_7 Depth=1
	r7 = r4 << 2
	r7 += r0  #
	r6 = [r1+0] 
	r6 <<= r3
	[r7+0] |= r6
.LBB0_3:                                ;   in Loop: Header=BB0_7 Depth=1
	if (r3 == 0) goto .LBB0_6
; BB#4:                                 ;   in Loop: Header=BB0_7 Depth=1
	r6 = r4 + 1
	if (r6 > 5) goto .LBB0_6
; BB#5:                                 ;   in Loop: Header=BB0_7 Depth=1
	r7 = r4 << 2
	r7 += r0  #
	r6 = [r1+0] 
	r6 >>= r5
	[r7+4] |= r6
.LBB0_6:                                ;   in Loop: Header=BB0_7 Depth=1
	r1 += 4
	r4 += 1
	r2 += -1
.LBB0_7:                                ; =>This Inner Loop Header: Depth=1
	if (r2 != 0) goto .LBB0_1
; BB#8:
	{pc, r7-r4} = [sp++]
.Lfunc_end0:
	.size	shift, .Lfunc_end0-shift

	.p2align	1
	.type	bit,@function
bit:                                    ; @bit
; BB#0:
	r2 = r1 >> 5
	r2 = r2 << 2
	r0 = [r0+r2]
	r1 = r1 & 0x1F
	r0 >>= r1
	r0 = r0 & 0x1
	rts
.Lfunc_end1:
	.size	bit, .Lfunc_end1-bit

	.globl	sj6_fused
	.p2align	1
	.type	sj6_fused,@function
sj6_fused:                              ; @sj6_fused
; BB#0:
	[--sp] = {rets, r15-r4}
	sp += -108
	r9_r8 = d[sp+160]
                                        ; kill: %R3<def> %R3<kill> %D2<def>
	r13_r12 = r1_r0
	r5_r4 = r9_r8
	r5_r4 >>= 52
	r1 = 2047
	r14 = r4 & r1
	r4 = 0
                                        ; kill: %R2<def> %R2<kill> %D2<kill> %D2<def>
	if (r14 == 2047) goto .LBB2_59
; BB#1:
	r7_r6 = r3_r2
	r7_r6 >>= 52
	r11 = r6 & r1
	if (r11 == 2047) goto .LBB2_59
; BB#2:
	r7_r6 = r13_r12
	r7_r6 >>= 52
	r15 = r6 & r1
	if (r15 == 2047) goto .LBB2_59
; BB#3:
	r1 = r3 | r13
	r0 = r2 | r12
	ifs (r1 < 0) goto .LBB2_59
; BB#4:
	if (r15 == 0) goto .LBB2_59
; BB#5:
	r5 = [sp+168]
	if (r11 == 0) goto .LBB2_31
; BB#6:
	r6 = 1048575
	r1 = r9 & r6
	r0 = r8
	r0 |= r1
	if (r0 == 0) goto .LBB2_8
; BB#7:
	if (r14 == 0) goto .LBB2_59
.LBB2_8:
	r0 = r13 & r6  #
	[sp+4] = r5                     ; 4-byte Folded Spill
	[sp+100] = r12
	r0 |= 1048576
	r0 = r3 & r6  #
	[sp+104] = r0
	[sp+92] = r2
	r0 |= 1048576
	r1_r0 = r9_r8  #
	[sp+96] = r0
	r8 = 0
	r9 = 0
	d[sp+84] = r9_r8
	d[sp+76] = r9_r8
	[sp+68] = r0
	d[sp+8] = r1_r0                 ; 8-byte Folded Spill
	r0 = r1 & r6
	r0 |= 1048576
	[sp+72] = r0
	d[sp+60] = r9_r8
	d[sp+52] = r9_r8
	d[sp+44] = r9_r8
	d[sp+36] = r9_r8
	d[sp+28] = r9_r8
	d[sp+20] = r9_r8
	r1 = 0
	goto .LBB2_10
.LBB2_9:                                ;   in Loop: Header=BB2_10 Depth=1
	r2 = sp + 100
	r12 = [r2+r4<<2]
.LBB2_10:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_11 Depth 2
	r13 = r1
	r7 = r1
	r2 = r8
	r3 = 0
.LBB2_11:                               ;   Parent Loop BB2_10 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	r6 = sp + 76
	r6 += r7
	r0 = [r6+r4<<2]
	r10 = sp + 92
	r5 = [r10+r7]
	r2 += r0
	r3 = r3 + r1 + c
	r3_r2 += r5 * r12 (u)
	[r6+r4<<2] = r2
	r7 += 4
	r3_r2 >>= 32
	if (r7 != 8) goto .LBB2_11
; BB#12:                                ;   in Loop: Header=BB2_10 Depth=1
	r7 = sp + 76
	r7 += 8
	[r7+r4<<2] = r2
	r4 += 1
	if (r4 != 2) goto .LBB2_9
; BB#13:
	r0 = r15 + r11
	r0 = r0 + -2150
	r1 = r14 + -1075
	if (r14 == 0) {
		r13 = r0
	} else {
		r13 = smin(r1, r0)
	}
	r9 = 1048575
	r3 = r0 - r13
	r8 = r1 - r13
	if (r14 == 0) {
		r8 = r14
	}
	r4 = 0
	if (r3 > 85) goto .LBB2_59
; BB#14:
	if (r8 > 138) goto .LBB2_59
; BB#15:
	r0 = sp + 44
	r1 = sp + 76
	r2 = 4
	call shift
	if (r14 == 0) goto .LBB2_33
; BB#16:
	r0 = sp + 20
	r1 = sp + 68
	r2 = 2
	r3 = r8
	call shift
	r1_r0 = d[sp+8]                 ; 8-byte Folded Reload
	r0 = r1 >> 31
	if (r0 == 0) goto .LBB2_33
; BB#17:
	r1_r0 = d[sp+8]                 ; 8-byte Folded Reload
	r1_r0 >>= 63
	d[sp+8] = r1_r0                 ; 8-byte Folded Spill
	r1_r0 = 0
	r8 = [sp+4]                     ; 4-byte Folded Reload
.LBB2_18:                               ; =>This Inner Loop Header: Depth=1
	r2 = sp + 20
	r2 = [r2+r0<<2]
	r3 = sp + 44
	r3 = [r3+r0<<2]
	r4 = 1
	if (r3 < r2) goto .LBB2_22
; BB#19:                                ;   in Loop: Header=BB2_18 Depth=1
	r5 = 1
	if (r3 != r2) {
		r5 = 0
	}
	if (r1 != 0) goto .LBB2_21
; BB#20:                                ;   in Loop: Header=BB2_18 Depth=1
	r4 = r1
.LBB2_21:                               ;   in Loop: Header=BB2_18 Depth=1
	r4 &= r5
.LBB2_22:                               ;   in Loop: Header=BB2_18 Depth=1
	r1 += r2
	r1 = r3 - r1
	r2 = sp + 44
	[r2+r0<<2] = r1
	r0 += 1
	r1 = r4
	if (r0 != 6) goto .LBB2_18
; BB#23:
	if (r4 == 0) goto .LBB2_37
; BB#24:
	r1 = 1
	r0 = 0
.LBB2_25:                               ; =>This Inner Loop Header: Depth=1
	r2 = sp + 44
	r3 = [r2+r0<<2]
	r3 = ~r3
	r3 += r1
	[r2+r0<<2] = r3
	r2 = 1
	if (r3 == 0) goto .LBB2_27
; BB#26:                                ;   in Loop: Header=BB2_25 Depth=1
	r2 = 0
.LBB2_27:                               ;   in Loop: Header=BB2_25 Depth=1
	r3 = 1
	if (r1 != 0) goto .LBB2_29
; BB#28:                                ;   in Loop: Header=BB2_25 Depth=1
	r3 = r1
.LBB2_29:                               ;   in Loop: Header=BB2_25 Depth=1
	r0 += 1
	r1 = r3 & r2
	if (r0 != 6) goto .LBB2_25
; BB#30:
	r1_r0 = d[sp+8]                 ; 8-byte Folded Reload
	r1_r0 <<= 63
	goto .LBB2_38
.LBB2_31:
	r0 = r2 | r3
	if (r0 != 0) goto .LBB2_59
; BB#32:
	d[r5+0] = r9_r8
	goto .LBB2_58
.LBB2_33:
	r1_r0 = 0
	r2 = 0
.LBB2_34:                               ; =>This Inner Loop Header: Depth=1
	r3 = sp + 44
	r4 = [r3+r2<<2]
	r5 = r1
	r6 = r4 + r0
	r7 = r5 + r1 + c
	r4 = sp + 20
	r0 = [r4+r2<<2]
	r4 = r6 + r0
	r5 = r7 + r1 + c
	[r3+r2<<2] = r4
	r2 += 1
	r0 = r5
	if (r2 != 6) goto .LBB2_34
; BB#35:
	r4 = 0
	if (r0 != 0) goto .LBB2_59
; BB#36:
	r0 = 0
	r1 = 0
	d[sp+8] = r1_r0                 ; 8-byte Folded Spill
	r8 = [sp+4]                     ; 4-byte Folded Reload
	goto .LBB2_39
.LBB2_37:
	r0 = 0
	r1 = 0
.LBB2_38:
	d[sp+8] = r1_r0                 ; 8-byte Folded Spill
.LBB2_39:
	r0 = sp + 44
	r0 += 20
	r7 = 224
.LBB2_40:                               ; =>This Inner Loop Header: Depth=1
	if (r7 == 32) goto .LBB2_57
; BB#41:                                ;   in Loop: Header=BB2_40 Depth=1
	r1 = [r0++=-4] 
	r7 += -32
	if (r1 == 0) goto .LBB2_40
; BB#42:
	r0 = clz(r1)
	r6 = r7 - r0
	if (r6 == 0) goto .LBB2_57
; BB#43:
	r1 = r13 - r0
	r1 += r7
	r0 = 0xBF - r0  #
	[sp] = r1                       ; 4-byte Folded Spill
	r10 = r0
	r11 = r0 + r7
	r3 = r11 + -192
	r5 = 0
	r14 = 0
	r15 = 0
	r12 = 52
	r8 = 0
	goto .LBB2_45
.LBB2_44:                               ;   in Loop: Header=BB2_45 Depth=1
	r0 = sp + 44
	r1 = r3
	call bit
	r4 = r0
	r1 = r5
	r1_r0 <<= r12
	r15 = r15 | r1
	r14 = r14 | r0
	r12 = r12 + -1
	r3 += -1
	r8 = r8 + 1
.LBB2_45:                               ; =>This Inner Loop Header: Depth=1
	if (r8 > 52) goto .LBB2_47
; BB#46:                                ;   in Loop: Header=BB2_45 Depth=1
	if (r8 < r6) goto .LBB2_44
.LBB2_47:
	r0 = r13 + r10
	r0 += r7
	r5 = r0 + -192
	if (r6 <= 53) goto .LBB2_55
; BB#48:
	r3 = r11 + -245
	r0 = sp + 44
	r1 = r3
	call bit
	if (r0 == 0) goto .LBB2_55
; BB#49:
	r0 = r14 & 0x1
	r4 = 0
	goto .LBB2_52
.LBB2_50:                               ;   in Loop: Header=BB2_52 Depth=1
	if (r4 >= r3) goto .LBB2_53
; BB#51:                                ;   in Loop: Header=BB2_52 Depth=1
	r0 = sp + 44
	r1 = r4
	call bit
                                        ; kill: %R0<def> %R0<kill> %D0<def>
	r4 += 1
.LBB2_52:                               ; =>This Inner Loop Header: Depth=1
	if (r0 == 0) goto .LBB2_50
.LBB2_53:
	r1 = 0
	r14 += r0
	r15 = r15 + r1 + c
	r0 = r15 ^ 0x200000
	r0 = r0 | r14
	if (r0 != 0) goto .LBB2_55
; BB#54:
	r14 = 0  #
	r5 = [sp]                       ; 4-byte Folded Reload
	r15 = r1 + 0x100000
.LBB2_55:
	r0 = r5 + 1022
	r4 = 0
	if (r0 > 2045) goto .LBB2_59
; BB#56:
	r15 = r15 & r9
	r3_r2 = d[sp+8]                 ; 8-byte Folded Reload
	r1 = r15 | r3
	r0 = r14 | r2
	r2 = r5 + 1023
	r3 = 0
	r3_r2 <<= 52
	r5 = r1 | r3
	r4 = r0 | r2  #
	r0 = [sp+4]                     ; 4-byte Folded Reload
	d[r0+0] = r5_r4
	goto .LBB2_58
.LBB2_57:
	r0 = 0
	r1 = 0
	d[r8+0] = r1_r0
.LBB2_58:
	r4 = 1
.LBB2_59:
	r0 = r4
	sp += 108
	{pc, r15-r4} = [sp++]
.Lfunc_end2:
	.size	sj6_fused, .Lfunc_end2-sj6_fused


