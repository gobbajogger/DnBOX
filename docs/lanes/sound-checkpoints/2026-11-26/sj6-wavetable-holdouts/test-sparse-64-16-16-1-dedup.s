	.text
	.file	"target-leaf.c"
	.globl	sj6_family_hot
	.p2align	1
	.type	sj6_family_hot,@function
sj6_family_hot:                         ; @sj6_family_hot
; BB#0:
	[--sp] = {rets, r12-r4}
	r3 = 0x42800000
	r4 = r2 * r3 (f)
	r6 = 1056629064
	r5 = 0
	if (r4 < r6) goto .LBB0_4
; BB#1:
	r5 = -1056629064
	r5 += r4
	r5 = r5 >> 23
	r5 += 1
	if (r5 <= 5) goto .LBB0_3
; BB#2:
	r11 = 0
	r5 = 6
	goto .LBB0_7
.LBB0_3:
	r6 = r5 << 23
	r4 = r4 - r6
.LBB0_4:
	r6 = 1053609165
	iff (r4 u<= r6) goto .LBB0_6
; BB#5:
	r6 = r6 + 0x80000000
	r4 = r4 + r6 (f)
	r6 = 1035489772
	r6 = r4 / r6 (f)
	r11 = r3 + 0xFD000000
	iff (r6 u<= r11) {
		r11 = r6
	}
	goto .LBB0_7
.LBB0_6:
	r11 = 0
.LBB0_7:
	r6 = -1082620450
	r2 = r2 * r6 (f)
	r2 = r1 + r2 (f)
	r1 = r3 + 0xFD000000
	r1 = r2 + r1 (f)
	r8 = 0
	iff (r2 u>= r8) {
		r1 = r2
	}
	r2 = r0 * 0x7
	r5 += r2
	r9 = TBL_IDX
	r2 = b[r5+r9] (u)
	r1 = r1 * r3 (f)
	r7 = ftou(r1) (trunc)
	r12 = r7 & 0x3F
	r2 = r2 << 7
	r10 = TBL
	r2 += r10
	r4 = h[r2+r12<<1] (s)
	r6 = r7 + 1
	r6 = r6 & 0x3F
	r3 = h[r2+r6<<1] (s)
	r2 = utof(r7)
	r2 = r1 - r2 (f)
	r1 = itof(r4)
	r3 = itof(r3)
	r3 = r3 - r1 (f)
	r3 = r2 * r3 (f)
	r1 = r1 + r3 (f)
	iff (r11 u<= r8) goto .LBB0_9
; BB#8:
	r3 = r5 + r9
	r3 = b[r3+1] (u)
	r3 = r3 << 7
	r3 += r10
	r4 = h[r3+r12<<1] (s)
	r3 = h[r3+r6<<1] (s)
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
	{pc, r12-r4} = [sp++]
.Lfunc_end0:
	.size	sj6_family_hot, .Lfunc_end0-sj6_family_hot

	.type	TBL,@object             ; @TBL
	.section	.rodata,"a",@progbits
	.p2align	1
TBL:
	.short	31181                   ; 0x79cd
	.short	31235                   ; 0x7a03
	.short	31069                   ; 0x795d
	.short	30669                   ; 0x77cd
	.short	30020                   ; 0x7544
	.short	29104                   ; 0x71b0
	.short	27903                   ; 0x6cff
	.short	26401                   ; 0x6721
	.short	24586                   ; 0x600a
	.short	22455                   ; 0x57b7
	.short	20012                   ; 0x4e2c
	.short	17271                   ; 0x4377
	.short	14258                   ; 0x37b2
	.short	11008                   ; 0x2b00
	.short	7565                    ; 0x1d8d
	.short	3984                    ; 0xf90
	.short	325                     ; 0x145
	.short	62185                   ; 0xf2e9
	.short	58558                   ; 0xe4be
	.short	55043                   ; 0xd703
	.short	51695                   ; 0xc9ef
	.short	48566                   ; 0xbdb6
	.short	45698                   ; 0xb282
	.short	43122                   ; 0xa872
	.short	40859                   ; 0x9f9b
	.short	38921                   ; 0x9809
	.short	37308                   ; 0x91bc
	.short	36015                   ; 0x8caf
	.short	35029                   ; 0x88d5
	.short	34335                   ; 0x861f
	.short	33915                   ; 0x847b
	.short	33753                   ; 0x83d9
	.short	33834                   ; 0x842a
	.short	34148                   ; 0x8564
	.short	34688                   ; 0x8780
	.short	35453                   ; 0x8a7d
	.short	36445                   ; 0x8e5d
	.short	37668                   ; 0x9324
	.short	39129                   ; 0x98d9
	.short	40833                   ; 0x9f81
	.short	42784                   ; 0xa720
	.short	44982                   ; 0xafb6
	.short	47418                   ; 0xb93a
	.short	50079                   ; 0xc39f
	.short	52943                   ; 0xcecf
	.short	55980                   ; 0xdaac
	.short	59154                   ; 0xe712
	.short	62420                   ; 0xf3d4
	.short	196                     ; 0xc4
	.short	3503                    ; 0xdaf
	.short	6756                    ; 0x1a64
	.short	9907                    ; 0x26b3
	.short	12912                   ; 0x3270
	.short	15734                   ; 0x3d76
	.short	18342                   ; 0x47a6
	.short	20716                   ; 0x50ec
	.short	22842                   ; 0x593a
	.short	24715                   ; 0x608b
	.short	26334                   ; 0x66de
	.short	27707                   ; 0x6c3b
	.short	28842                   ; 0x70aa
	.short	29749                   ; 0x7435
	.short	30438                   ; 0x76e6
	.short	30915                   ; 0x78c3
	.short	32229                   ; 0x7de5
	.short	32257                   ; 0x7e01
	.short	31976                   ; 0x7ce8
	.short	31383                   ; 0x7a97
	.short	30481                   ; 0x7711
	.short	29271                   ; 0x7257
	.short	27762                   ; 0x6c72
	.short	25963                   ; 0x656b
	.short	23891                   ; 0x5d53
	.short	21561                   ; 0x5439
	.short	18997                   ; 0x4a35
	.short	16223                   ; 0x3f5f
	.short	13266                   ; 0x33d2
	.short	10157                   ; 0x27ad
	.short	6930                    ; 0x1b12
	.short	3619                    ; 0xe23
	.short	260                     ; 0x104
	.short	62428                   ; 0xf3dc
	.short	59087                   ; 0xe6cf
	.short	55812                   ; 0xda04
	.short	52638                   ; 0xcd9e
	.short	49603                   ; 0xc1c3
	.short	46738                   ; 0xb692
	.short	44076                   ; 0xac2c
	.short	41645                   ; 0xa2ad
	.short	39471                   ; 0x9a2f
	.short	37575                   ; 0x92c7
	.short	35976                   ; 0x8c88
	.short	34687                   ; 0x877f
	.short	33720                   ; 0x83b8
	.short	33080                   ; 0x8138
	.short	32769                   ; 0x8001
	.short	32786                   ; 0x8012
	.short	33127                   ; 0x8167
	.short	33782                   ; 0x83f6
	.short	34739                   ; 0x87b3
	.short	35984                   ; 0x8c90
	.short	37501                   ; 0x927d
	.short	39270                   ; 0x9966
	.short	41270                   ; 0xa136
	.short	43480                   ; 0xa9d8
	.short	45875                   ; 0xb333
	.short	48433                   ; 0xbd31
	.short	51128                   ; 0xc7b8
	.short	53935                   ; 0xd2af
	.short	56831                   ; 0xddff
	.short	59789                   ; 0xe98d
	.short	62786                   ; 0xf542
	.short	260                     ; 0x104
	.short	3261                    ; 0xcbd
	.short	6228                    ; 0x1854
	.short	9138                    ; 0x23b2
	.short	11969                   ; 0x2ec1
	.short	14697                   ; 0x3969
	.short	17302                   ; 0x4396
	.short	19762                   ; 0x4d32
	.short	22056                   ; 0x5628
	.short	24164                   ; 0x5e64
	.short	26067                   ; 0x65d3
	.short	27746                   ; 0x6c62
	.short	29184                   ; 0x7200
	.short	30364                   ; 0x769c
	.short	31274                   ; 0x7a2a
	.short	31899                   ; 0x7c9b
	.short	32489                   ; 0x7ee9
	.short	32333                   ; 0x7e4d
	.short	31865                   ; 0x7c79
	.short	31090                   ; 0x7972
	.short	30016                   ; 0x7540
	.short	28653                   ; 0x6fed
	.short	27014                   ; 0x6986
	.short	25115                   ; 0x621b
	.short	22973                   ; 0x59bd
	.short	20611                   ; 0x5083
	.short	18050                   ; 0x4682
	.short	15315                   ; 0x3bd3
	.short	12433                   ; 0x3091
	.short	9431                    ; 0x24d7
	.short	6338                    ; 0x18c2
	.short	3185                    ; 0xc71
	.short	0                       ; 0x0
	.short	62351                   ; 0xf38f
	.short	59198                   ; 0xe73e
	.short	56105                   ; 0xdb29
	.short	53103                   ; 0xcf6f
	.short	50221                   ; 0xc42d
	.short	47486                   ; 0xb97e
	.short	44925                   ; 0xaf7d
	.short	42563                   ; 0xa643
	.short	40421                   ; 0x9de5
	.short	38522                   ; 0x967a
	.short	36883                   ; 0x9013
	.short	35520                   ; 0x8ac0
	.short	34446                   ; 0x868e
	.short	33671                   ; 0x8387
	.short	33203                   ; 0x81b3
	.short	33047                   ; 0x8117
	.short	33203                   ; 0x81b3
	.short	33671                   ; 0x8387
	.short	34446                   ; 0x868e
	.short	35520                   ; 0x8ac0
	.short	36883                   ; 0x9013
	.short	38522                   ; 0x967a
	.short	40421                   ; 0x9de5
	.short	42563                   ; 0xa643
	.short	44925                   ; 0xaf7d
	.short	47486                   ; 0xb97e
	.short	50221                   ; 0xc42d
	.short	53103                   ; 0xcf6f
	.short	56105                   ; 0xdb29
	.short	59198                   ; 0xe73e
	.short	62351                   ; 0xf38f
	.short	0                       ; 0x0
	.short	3185                    ; 0xc71
	.short	6338                    ; 0x18c2
	.short	9431                    ; 0x24d7
	.short	12433                   ; 0x3091
	.short	15315                   ; 0x3bd3
	.short	18050                   ; 0x4682
	.short	20611                   ; 0x5083
	.short	22973                   ; 0x59bd
	.short	25115                   ; 0x621b
	.short	27014                   ; 0x6986
	.short	28653                   ; 0x6fed
	.short	30016                   ; 0x7540
	.short	31090                   ; 0x7972
	.short	31865                   ; 0x7c79
	.short	32333                   ; 0x7e4d
	.short	28505                   ; 0x6f59
	.short	27458                   ; 0x6b42
	.short	25165                   ; 0x624d
	.short	23017                   ; 0x59e9
	.short	21528                   ; 0x5418
	.short	20150                   ; 0x4eb6
	.short	18451                   ; 0x4813
	.short	16672                   ; 0x4120
	.short	15086                   ; 0x3aee
	.short	13531                   ; 0x34db
	.short	11797                   ; 0x2e15
	.short	10004                   ; 0x2714
	.short	8317                    ; 0x207d
	.short	6640                    ; 0x19f0
	.short	4832                    ; 0x12e0
	.short	2968                    ; 0xb98
	.short	1174                    ; 0x496
	.short	64924                   ; 0xfd9c
	.short	63024                   ; 0xf630
	.short	61060                   ; 0xee84
	.short	59152                   ; 0xe710
	.short	57258                   ; 0xdfaa
	.short	55249                   ; 0xd7d1
	.short	53155                   ; 0xcfa3
	.short	51118                   ; 0xc7ae
	.short	49116                   ; 0xbfdc
	.short	46972                   ; 0xb77c
	.short	44690                   ; 0xae92
	.short	42503                   ; 0xa607
	.short	40423                   ; 0x9de7
	.short	38075                   ; 0x94bb
	.short	35367                   ; 0x8a27
	.short	33176                   ; 0x8198
	.short	32769                   ; 0x8001
	.short	34427                   ; 0x867b
	.short	37109                   ; 0x90f5
	.short	39664                   ; 0x9af0
	.short	41927                   ; 0xa3c7
	.short	44318                   ; 0xad1e
	.short	46886                   ; 0xb726
	.short	49309                   ; 0xc09d
	.short	51548                   ; 0xc95c
	.short	53863                   ; 0xd267
	.short	56276                   ; 0xdbd4
	.short	58550                   ; 0xe4b6
	.short	60666                   ; 0xecfa
	.short	62844                   ; 0xf57c
	.short	65092                   ; 0xfe44
	.short	1655                    ; 0x677
	.short	3605                    ; 0xe15
	.short	5621                    ; 0x15f5
	.short	7695                    ; 0x1e0f
	.short	9601                    ; 0x2581
	.short	11354                   ; 0x2c5a
	.short	13196                   ; 0x338c
	.short	15098                   ; 0x3afa
	.short	16791                   ; 0x4197
	.short	18315                   ; 0x478b
	.short	19982                   ; 0x4e0e
	.short	21734                   ; 0x54e6
	.short	23175                   ; 0x5a87
	.short	24382                   ; 0x5f3e
	.short	25920                   ; 0x6540
	.short	27689                   ; 0x6c29
	.short	27597                   ; 0x6bcd
	.short	26915                   ; 0x6923
	.short	25553                   ; 0x63d1
	.short	23755                   ; 0x5ccb
	.short	21791                   ; 0x551f
	.short	19879                   ; 0x4da7
	.short	18130                   ; 0x46d2
	.short	16547                   ; 0x40a3
	.short	15053                   ; 0x3acd
	.short	13546                   ; 0x34ea
	.short	11944                   ; 0x2ea8
	.short	10217                   ; 0x27e9
	.short	8390                    ; 0x20c6
	.short	6521                    ; 0x1979
	.short	4671                    ; 0x123f
	.short	2878                    ; 0xb3e
	.short	1137                    ; 0x471
	.short	64939                   ; 0xfdab
	.short	63150                   ; 0xf6ae
	.short	61253                   ; 0xef45
	.short	59238                   ; 0xe766
	.short	57143                   ; 0xdf37
	.short	55044                   ; 0xd704
	.short	53012                   ; 0xcf14
	.short	51074                   ; 0xc782
	.short	49180                   ; 0xc01c
	.short	47215                   ; 0xb86f
	.short	45047                   ; 0xaff7
	.short	42612                   ; 0xa674
	.short	39980                   ; 0x9c2c
	.short	37391                   ; 0x920f
	.short	35212                   ; 0x898c
	.short	33839                   ; 0x842f
	.short	33566                   ; 0x831e
	.short	34481                   ; 0x86b1
	.short	36426                   ; 0x8e4a
	.short	39051                   ; 0x988b
	.short	41933                   ; 0xa3cd
	.short	44717                   ; 0xaead
	.short	47216                   ; 0xb870
	.short	49441                   ; 0xc121
	.short	51534                   ; 0xc94e
	.short	53671                   ; 0xd1a7
	.short	55956                   ; 0xda94
	.short	58374                   ; 0xe406
	.short	60814                   ; 0xed8e
	.short	63138                   ; 0xf6a2
	.short	65264                   ; 0xfef0
	.short	1671                    ; 0x687
	.short	3530                    ; 0xdca
	.short	5427                    ; 0x1533
	.short	7432                    ; 0x1d08
	.short	9523                    ; 0x2533
	.short	11593                   ; 0x2d49
	.short	13515                   ; 0x34cb
	.short	15217                   ; 0x3b71
	.short	16726                   ; 0x4156
	.short	18165                   ; 0x46f5
	.short	19687                   ; 0x4ce7
	.short	21388                   ; 0x538c
	.short	23235                   ; 0x5ac3
	.short	25049                   ; 0x61d9
	.short	26554                   ; 0x67ba
	.short	27466                   ; 0x6b4a
	.short	26166                   ; 0x6636
	.short	25726                   ; 0x647e
	.short	24942                   ; 0x616e
	.short	23851                   ; 0x5d2b
	.short	22498                   ; 0x57e2
	.short	20932                   ; 0x51c4
	.short	19202                   ; 0x4b02
	.short	17357                   ; 0x43cd
	.short	15442                   ; 0x3c52
	.short	13496                   ; 0x34b8
	.short	11556                   ; 0x2d24
	.short	9649                    ; 0x25b1
	.short	7796                    ; 0x1e74
	.short	6009                    ; 0x1779
	.short	4289                    ; 0x10c1
	.short	2629                    ; 0xa45
	.short	1010                    ; 0x3f2
	.short	64938                   ; 0xfdaa
	.short	63308                   ; 0xf74c
	.short	61615                   ; 0xf0af
	.short	59821                   ; 0xe9ad
	.short	57892                   ; 0xe224
	.short	55809                   ; 0xda01
	.short	53565                   ; 0xd13d
	.short	51176                   ; 0xc7e8
	.short	48683                   ; 0xbe2b
	.short	46149                   ; 0xb445
	.short	43656                   ; 0xaa88
	.short	41305                   ; 0xa159
	.short	39205                   ; 0x9925
	.short	37467                   ; 0x925b
	.short	36189                   ; 0x8d5d
	.short	35453                   ; 0x8a7d
	.short	35312                   ; 0x89f0
	.short	35789                   ; 0x8bcd
	.short	36868                   ; 0x9004
	.short	38501                   ; 0x9665
	.short	40607                   ; 0x9e9f
	.short	43081                   ; 0xa849
	.short	45806                   ; 0xb2ee
	.short	48659                   ; 0xbe13
	.short	51526                   ; 0xc946
	.short	54308                   ; 0xd424
	.short	56934                   ; 0xde66
	.short	59358                   ; 0xe7de
	.short	61570                   ; 0xf082
	.short	63583                   ; 0xf85f
	.short	65437                   ; 0xff9d
	.short	1651                    ; 0x673
	.short	3358                    ; 0xd1e
	.short	5081                    ; 0x13d9
	.short	6867                    ; 0x1ad3
	.short	8749                    ; 0x222d
	.short	10733                   ; 0x29ed
	.short	12806                   ; 0x3206
	.short	14930                   ; 0x3a52
	.short	17052                   ; 0x429c
	.short	19104                   ; 0x4aa0
	.short	21012                   ; 0x5214
	.short	22705                   ; 0x58b1
	.short	24117                   ; 0x5e35
	.short	25197                   ; 0x626d
	.short	25908                   ; 0x6534
	.short	26231                   ; 0x6677
	.short	23684                   ; 0x5c84
	.short	23392                   ; 0x5b60
	.short	22927                   ; 0x598f
	.short	22297                   ; 0x5719
	.short	21508                   ; 0x5404
	.short	20566                   ; 0x5056
	.short	19477                   ; 0x4c15
	.short	18246                   ; 0x4746
	.short	16878                   ; 0x41ee
	.short	15377                   ; 0x3c11
	.short	13747                   ; 0x35b3
	.short	11995                   ; 0x2edb
	.short	10126                   ; 0x278e
	.short	8146                    ; 0x1fd2
	.short	6066                    ; 0x17b2
	.short	3894                    ; 0xf36
	.short	1645                    ; 0x66d
	.short	64868                   ; 0xfd64
	.short	62509                   ; 0xf42d
	.short	60124                   ; 0xeadc
	.short	57736                   ; 0xe188
	.short	55368                   ; 0xd848
	.short	53047                   ; 0xcf37
	.short	50801                   ; 0xc671
	.short	48658                   ; 0xbe12
	.short	46648                   ; 0xb638
	.short	44800                   ; 0xaf00
	.short	43143                   ; 0xa887
	.short	41703                   ; 0xa2e7
	.short	40504                   ; 0x9e38
	.short	39570                   ; 0x9a92
	.short	38918                   ; 0x9806
	.short	38563                   ; 0x96a3
	.short	38515                   ; 0x9673
	.short	38780                   ; 0x977c
	.short	39358                   ; 0x99be
	.short	40243                   ; 0x9d33
	.short	41427                   ; 0xa1d3
	.short	42894                   ; 0xa78e
	.short	44624                   ; 0xae50
	.short	46594                   ; 0xb602
	.short	48777                   ; 0xbe89
	.short	51140                   ; 0xc7c4
	.short	53652                   ; 0xd194
	.short	56276                   ; 0xdbd4
	.short	58978                   ; 0xe662
	.short	61719                   ; 0xf117
	.short	64465                   ; 0xfbd1
	.short	1645                    ; 0x66d
	.short	4297                    ; 0x10c9
	.short	6856                    ; 0x1ac8
	.short	9293                    ; 0x244d
	.short	11585                   ; 0x2d41
	.short	13711                   ; 0x358f
	.short	15654                   ; 0x3d26
	.short	17401                   ; 0x43f9
	.short	18942                   ; 0x49fe
	.short	20270                   ; 0x4f2e
	.short	21384                   ; 0x5388
	.short	22282                   ; 0x570a
	.short	22967                   ; 0x59b7
	.short	23444                   ; 0x5b94
	.short	23717                   ; 0x5ca5
	.short	23795                   ; 0x5cf3
	.short	25328                   ; 0x62f0
	.short	25206                   ; 0x6276
	.short	24842                   ; 0x610a
	.short	24238                   ; 0x5eae
	.short	23400                   ; 0x5b68
	.short	22338                   ; 0x5742
	.short	21060                   ; 0x5244
	.short	19579                   ; 0x4c7b
	.short	17910                   ; 0x45f6
	.short	16068                   ; 0x3ec4
	.short	14072                   ; 0x36f8
	.short	11940                   ; 0x2ea4
	.short	9693                    ; 0x25dd
	.short	7352                    ; 0x1cb8
	.short	4941                    ; 0x134d
	.short	2483                    ; 0x9b3
	.short	0                       ; 0x0
	.short	63053                   ; 0xf64d
	.short	60595                   ; 0xecb3
	.short	58184                   ; 0xe348
	.short	55843                   ; 0xda23
	.short	53596                   ; 0xd15c
	.short	51464                   ; 0xc908
	.short	49468                   ; 0xc13c
	.short	47626                   ; 0xba0a
	.short	45957                   ; 0xb385
	.short	44476                   ; 0xadbc
	.short	43198                   ; 0xa8be
	.short	42136                   ; 0xa498
	.short	41298                   ; 0xa152
	.short	40694                   ; 0x9ef6
	.short	40330                   ; 0x9d8a
	.short	40208                   ; 0x9d10
	.short	40330                   ; 0x9d8a
	.short	40694                   ; 0x9ef6
	.short	41298                   ; 0xa152
	.short	42136                   ; 0xa498
	.short	43198                   ; 0xa8be
	.short	44476                   ; 0xadbc
	.short	45957                   ; 0xb385
	.short	47626                   ; 0xba0a
	.short	49468                   ; 0xc13c
	.short	51464                   ; 0xc908
	.short	53596                   ; 0xd15c
	.short	55843                   ; 0xda23
	.short	58184                   ; 0xe348
	.short	60595                   ; 0xecb3
	.short	63053                   ; 0xf64d
	.short	0                       ; 0x0
	.short	2483                    ; 0x9b3
	.short	4941                    ; 0x134d
	.short	7352                    ; 0x1cb8
	.short	9693                    ; 0x25dd
	.short	11940                   ; 0x2ea4
	.short	14072                   ; 0x36f8
	.short	16068                   ; 0x3ec4
	.short	17910                   ; 0x45f6
	.short	19579                   ; 0x4c7b
	.short	21060                   ; 0x5244
	.short	22338                   ; 0x5742
	.short	23400                   ; 0x5b68
	.short	24238                   ; 0x5eae
	.short	24842                   ; 0x610a
	.short	25206                   ; 0x6276
	.size	TBL, 1024

	.type	TBL_IDX,@object         ; @TBL_IDX
TBL_IDX:
	.ascii	"\000\000\000\000\000\001\002"
	.ascii	"\003\003\003\004\005\006\007"
	.size	TBL_IDX, 14

	.type	TBL_GAIN,@object        ; @TBL_GAIN
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	2
TBL_GAIN:
	.long	939602863               ; float 3.08041308E-5
	.long	941996840               ; float 3.95133684E-5
	.size	TBL_GAIN, 8


