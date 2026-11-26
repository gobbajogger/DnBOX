	.text
	.file	"target-leaf.c"
	.globl	sj6_family_hot
	.p2align	1
	.type	sj6_family_hot,@function
sj6_family_hot:                         ; @sj6_family_hot
; BB#0:
	[--sp] = {rets, r11-r4}
	r3 = 0x42800000
	r3 = r2 * r3 (f)
	r5 = 1056629064
	r4 = 0
	if (r3 < r5) goto .LBB0_4
; BB#1:
	r4 = -1056629064
	r4 += r3
	r4 = r4 >> 23
	r4 += 1
	if (r4 <= 5) goto .LBB0_3
; BB#2:
	r11 = 0
	r4 = 6
	goto .LBB0_7
.LBB0_3:
	r5 = r4 << 23
	r3 = r3 - r5
.LBB0_4:
	r5 = 1053609165
	iff (r3 u<= r5) goto .LBB0_6
; BB#5:
	r5 = r5 + 0x80000000
	r3 = r3 + r5 (f)
	r5 = 1035489772
	r5 = r3 / r5 (f)
	r11 = 0x3F800000
	iff (r5 u<= r11) {
		r11 = r5
	}
	goto .LBB0_7
.LBB0_6:
	r11 = 0
.LBB0_7:
	r5 = -1082620450
	r2 = r2 * r5 (f)
	r2 = r1 + r2 (f)
	r1 = 0x3F800000
	r1 = r2 + r1 (f)
	r8 = 0
	iff (r2 u>= r8) {
		r1 = r2
	}
	r2 = r8 + 0x42000000
	r5 = r0 * 0x7
	r5 += r4
	r9 = TBL_IDX
	r6 = b[r5+r9] (u)
	r1 = r1 * r2 (f)
	r2 = ftou(r1) (trunc)
	r4 = r2 & 0x1F
	r6 = r6 << 5
	r10 = TBL
	r7 = r6 + r10
	r3 = b[r7+r4] (s)
	r6 = r2 + 1
	r6 = r6 & 0x1F
	r7 = b[r7+r6] (s)
	r2 = utof(r2)
	r2 = r1 - r2 (f)
	r1 = itof(r3)
	r3 = itof(r7)
	r3 = r3 - r1 (f)
	r3 = r2 * r3 (f)
	r1 = r1 + r3 (f)
	iff (r11 u<= r8) goto .LBB0_9
; BB#8:
	r3 = r5 + r9
	r3 = b[r3+1] (u)
	r3 = r3 << 5
	r3 += r10
	r4 = b[r3+r4] (s)
	r3 = b[r3+r6] (s)
	r4 = itof(r4)
	r3 = itof(r3)
	r3 = r3 - r4 (f)
	r2 = r2 * r3 (f)
	r2 = r4 + r2 (f)
	r2 = r2 - r1 (f)
	r2 = r11 * r2 (f)
	r1 = r1 + r2 (f)
.LBB0_9:
	if (r0 == 0) {
		r2 = 1058959343
	} else {
		r2 = 1056057543
	}
	r3 = TBL_GAIN
	r0 = [r3+r0<<2]
	r0 = r2 * r0 (f)
	r0 = r1 * r0 (f)
	{pc, r11-r4} = [sp++]
.Lfunc_end0:
	.size	sj6_family_hot, .Lfunc_end0-sj6_family_hot

	.type	TBL,@object             ; @TBL
	.section	.rodata,"a",@progbits
TBL:
	.ascii	"yxtl_N7\035\001\345\312\263\240\222\212\205\205\210\217\232\250\272\317\347\001\0322GYfpv"
	.ascii	"}|vl]J3\033\001\347\316\267\243\224\210\202\201\205\215\232\252\276\323\352\001\030.CVeqy"
	.ascii	"~|tiYF0\031\000\347\320\272\247\227\214\204\202\204\214\227\247\272\320\347\000\0310FYit|"
	.ascii	"ngVG<0!\022\005\367\347\326\307\270\245\217\201\204\227\255\300\321\343\366\007\026&5CO\\i"
	.ascii	"hcZL=.\037\021\004\367\351\332\307\263\240\220\210\211\224\247\275\324\350\370\007\024#3DT`g"
	.ascii	"^[VMC7(\030\007\364\341\316\275\255\241\231\225\225\233\246\265\307\333\361\007\033.>KU[^"
	.ascii	"ec]TG8'\024\000\354\331\310\271\254\243\235\233\235\243\254\271\310\331\354\000\024'8GT]c"
	.size	TBL, 224

	.type	TBL_IDX,@object         ; @TBL_IDX
TBL_IDX:
	.ascii	"\000\000\000\000\000\001\002"
	.ascii	"\003\003\003\003\004\005\006"
	.size	TBL_IDX, 14

	.type	TBL_GAIN,@object        ; @TBL_GAIN
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	2
TBL_GAIN:
	.long	1006794661              ; float 0.00796309579
	.long	1008926815              ; float 0.00994881894
	.size	TBL_GAIN, 8


