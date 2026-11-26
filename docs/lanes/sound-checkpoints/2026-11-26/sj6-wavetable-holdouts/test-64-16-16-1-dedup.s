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
	.short	31759                   ; 0x7c0f
	.short	31813                   ; 0x7c45
	.short	31626                   ; 0x7b8a
	.short	31190                   ; 0x79d6
	.short	30490                   ; 0x771a
	.short	29512                   ; 0x7348
	.short	28243                   ; 0x6e53
	.short	26669                   ; 0x682d
	.short	24780                   ; 0x60cc
	.short	22575                   ; 0x582f
	.short	20059                   ; 0x4e5b
	.short	17245                   ; 0x435d
	.short	14161                   ; 0x3751
	.short	10842                   ; 0x2a5a
	.short	7337                    ; 0x1ca9
	.short	3702                    ; 0xe76
	.short	0                       ; 0x0
	.short	61834                   ; 0xf18a
	.short	58199                   ; 0xe357
	.short	54694                   ; 0xd5a6
	.short	51375                   ; 0xc8af
	.short	48291                   ; 0xbca3
	.short	45477                   ; 0xb1a5
	.short	42961                   ; 0xa7d1
	.short	40756                   ; 0x9f34
	.short	38867                   ; 0x97d3
	.short	37293                   ; 0x91ad
	.short	36024                   ; 0x8cb8
	.short	35046                   ; 0x88e6
	.short	34346                   ; 0x862a
	.short	33910                   ; 0x8476
	.short	33723                   ; 0x83bb
	.short	33777                   ; 0x83f1
	.short	34064                   ; 0x8510
	.short	34580                   ; 0x8714
	.short	35326                   ; 0x89fe
	.short	36303                   ; 0x8dcf
	.short	37517                   ; 0x928d
	.short	38972                   ; 0x983c
	.short	40672                   ; 0x9ee0
	.short	42621                   ; 0xa67d
	.short	44814                   ; 0xaf0e
	.short	47245                   ; 0xb88d
	.short	49901                   ; 0xc2ed
	.short	52758                   ; 0xce16
	.short	55790                   ; 0xd9ee
	.short	58959                   ; 0xe64f
	.short	62223                   ; 0xf30f
	.short	0                       ; 0x0
	.short	3313                    ; 0xcf1
	.short	6577                    ; 0x19b1
	.short	9746                    ; 0x2612
	.short	12778                   ; 0x31ea
	.short	15635                   ; 0x3d13
	.short	18291                   ; 0x4773
	.short	20722                   ; 0x50f2
	.short	22915                   ; 0x5983
	.short	24864                   ; 0x6120
	.short	26564                   ; 0x67c4
	.short	28019                   ; 0x6d73
	.short	29233                   ; 0x7231
	.short	30210                   ; 0x7602
	.short	30956                   ; 0x78ec
	.short	31472                   ; 0x7af0
	.short	31743                   ; 0x7bff
	.short	31798                   ; 0x7c36
	.short	31617                   ; 0x7b81
	.short	31187                   ; 0x79d3
	.short	30496                   ; 0x7720
	.short	29525                   ; 0x7355
	.short	28259                   ; 0x6e63
	.short	26685                   ; 0x683d
	.short	24793                   ; 0x60d9
	.short	22581                   ; 0x5835
	.short	20056                   ; 0x4e58
	.short	17235                   ; 0x4353
	.short	14145                   ; 0x3741
	.short	10825                   ; 0x2a49
	.short	7323                    ; 0x1c9b
	.short	3694                    ; 0xe6e
	.short	0                       ; 0x0
	.short	61842                   ; 0xf192
	.short	58213                   ; 0xe365
	.short	54711                   ; 0xd5b7
	.short	51391                   ; 0xc8bf
	.short	48301                   ; 0xbcad
	.short	45480                   ; 0xb1a8
	.short	42955                   ; 0xa7cb
	.short	40743                   ; 0x9f27
	.short	38851                   ; 0x97c3
	.short	37277                   ; 0x919d
	.short	36011                   ; 0x8cab
	.short	35040                   ; 0x88e0
	.short	34349                   ; 0x862d
	.short	33919                   ; 0x847f
	.short	33738                   ; 0x83ca
	.short	33793                   ; 0x8401
	.short	34078                   ; 0x851e
	.short	34589                   ; 0x871d
	.short	35327                   ; 0x89ff
	.short	36297                   ; 0x8dc9
	.short	37504                   ; 0x9280
	.short	38956                   ; 0x982c
	.short	40657                   ; 0x9ed1
	.short	42609                   ; 0xa671
	.short	44809                   ; 0xaf09
	.short	47249                   ; 0xb891
	.short	49911                   ; 0xc2f7
	.short	52773                   ; 0xce25
	.short	55806                   ; 0xd9fe
	.short	58972                   ; 0xe65c
	.short	62230                   ; 0xf316
	.short	0                       ; 0x0
	.short	3306                    ; 0xcea
	.short	6564                    ; 0x19a4
	.short	9730                    ; 0x2602
	.short	12763                   ; 0x31db
	.short	15625                   ; 0x3d09
	.short	18287                   ; 0x476f
	.short	20727                   ; 0x50f7
	.short	22927                   ; 0x598f
	.short	24879                   ; 0x612f
	.short	26580                   ; 0x67d4
	.short	28032                   ; 0x6d80
	.short	29239                   ; 0x7237
	.short	30209                   ; 0x7601
	.short	30947                   ; 0x78e3
	.short	31458                   ; 0x7ae2
	.short	32743                   ; 0x7fe7
	.short	32767                   ; 0x7fff
	.short	32471                   ; 0x7ed7
	.short	31851                   ; 0x7c6b
	.short	30910                   ; 0x78be
	.short	29652                   ; 0x73d4
	.short	28086                   ; 0x6db6
	.short	26225                   ; 0x6671
	.short	24086                   ; 0x5e16
	.short	21687                   ; 0x54b7
	.short	19053                   ; 0x4a6d
	.short	16211                   ; 0x3f53
	.short	13190                   ; 0x3386
	.short	10023                   ; 0x2727
	.short	6745                    ; 0x1a59
	.short	3391                    ; 0xd3f
	.short	0                       ; 0x0
	.short	62145                   ; 0xf2c1
	.short	58791                   ; 0xe5a7
	.short	55513                   ; 0xd8d9
	.short	52346                   ; 0xcc7a
	.short	49325                   ; 0xc0ad
	.short	46483                   ; 0xb593
	.short	43849                   ; 0xab49
	.short	41450                   ; 0xa1ea
	.short	39311                   ; 0x998f
	.short	37450                   ; 0x924a
	.short	35884                   ; 0x8c2c
	.short	34626                   ; 0x8742
	.short	33685                   ; 0x8395
	.short	33065                   ; 0x8129
	.short	32769                   ; 0x8001
	.short	32793                   ; 0x8019
	.short	33133                   ; 0x816d
	.short	33780                   ; 0x83f4
	.short	34722                   ; 0x87a2
	.short	35945                   ; 0x8c69
	.short	37435                   ; 0x923b
	.short	39173                   ; 0x9905
	.short	41141                   ; 0xa0b5
	.short	43316                   ; 0xa934
	.short	45679                   ; 0xb26f
	.short	48207                   ; 0xbc4f
	.short	50877                   ; 0xc6bd
	.short	53666                   ; 0xd1a2
	.short	56550                   ; 0xdce6
	.short	59505                   ; 0xe871
	.short	62509                   ; 0xf42d
	.short	0                       ; 0x0
	.short	3027                    ; 0xbd3
	.short	6031                    ; 0x178f
	.short	8986                    ; 0x231a
	.short	11870                   ; 0x2e5e
	.short	14659                   ; 0x3943
	.short	17329                   ; 0x43b1
	.short	19857                   ; 0x4d91
	.short	22220                   ; 0x56cc
	.short	24395                   ; 0x5f4b
	.short	26363                   ; 0x66fb
	.short	28101                   ; 0x6dc5
	.short	29591                   ; 0x7397
	.short	30814                   ; 0x785e
	.short	31756                   ; 0x7c0c
	.short	32403                   ; 0x7e93
	.short	32743                   ; 0x7fe7
	.short	32585                   ; 0x7f49
	.short	32114                   ; 0x7d72
	.short	31333                   ; 0x7a65
	.short	30250                   ; 0x762a
	.short	28876                   ; 0x70cc
	.short	27225                   ; 0x6a59
	.short	25310                   ; 0x62de
	.short	23153                   ; 0x5a71
	.short	20772                   ; 0x5124
	.short	18191                   ; 0x470f
	.short	15435                   ; 0x3c4b
	.short	12530                   ; 0x30f2
	.short	9505                    ; 0x2521
	.short	6388                    ; 0x18f4
	.short	3209                    ; 0xc89
	.short	0                       ; 0x0
	.short	62327                   ; 0xf377
	.short	59148                   ; 0xe70c
	.short	56031                   ; 0xdadf
	.short	53006                   ; 0xcf0e
	.short	50101                   ; 0xc3b5
	.short	47345                   ; 0xb8f1
	.short	44764                   ; 0xaedc
	.short	42383                   ; 0xa58f
	.short	40226                   ; 0x9d22
	.short	38311                   ; 0x95a7
	.short	36660                   ; 0x8f34
	.short	35286                   ; 0x89d6
	.short	34203                   ; 0x859b
	.short	33422                   ; 0x828e
	.short	32951                   ; 0x80b7
	.short	32793                   ; 0x8019
	.short	32951                   ; 0x80b7
	.short	33422                   ; 0x828e
	.short	34203                   ; 0x859b
	.short	35286                   ; 0x89d6
	.short	36660                   ; 0x8f34
	.short	38311                   ; 0x95a7
	.short	40226                   ; 0x9d22
	.short	42383                   ; 0xa58f
	.short	44764                   ; 0xaedc
	.short	47345                   ; 0xb8f1
	.short	50101                   ; 0xc3b5
	.short	53006                   ; 0xcf0e
	.short	56031                   ; 0xdadf
	.short	59148                   ; 0xe70c
	.short	62327                   ; 0xf377
	.short	0                       ; 0x0
	.short	3209                    ; 0xc89
	.short	6388                    ; 0x18f4
	.short	9505                    ; 0x2521
	.short	12530                   ; 0x30f2
	.short	15435                   ; 0x3c4b
	.short	18191                   ; 0x470f
	.short	20772                   ; 0x5124
	.short	23153                   ; 0x5a71
	.short	25310                   ; 0x62de
	.short	27225                   ; 0x6a59
	.short	28876                   ; 0x70cc
	.short	30250                   ; 0x762a
	.short	31333                   ; 0x7a65
	.short	32114                   ; 0x7d72
	.short	32585                   ; 0x7f49
	.short	30900                   ; 0x78b4
	.short	28534                   ; 0x6f76
	.short	26664                   ; 0x6828
	.short	25011                   ; 0x61b3
	.short	23043                   ; 0x5a03
	.short	20941                   ; 0x51cd
	.short	19086                   ; 0x4a8e
	.short	17322                   ; 0x43aa
	.short	15346                   ; 0x3bf2
	.short	13306                   ; 0x33fa
	.short	11458                   ; 0x2cc2
	.short	9655                    ; 0x25b7
	.short	7670                    ; 0x1df6
	.short	5659                    ; 0x161b
	.short	3820                    ; 0xeec
	.short	1995                    ; 0x7cb
	.short	0                       ; 0x0
	.short	63541                   ; 0xf835
	.short	61716                   ; 0xf114
	.short	59877                   ; 0xe9e5
	.short	57866                   ; 0xe20a
	.short	55881                   ; 0xda49
	.short	54078                   ; 0xd33e
	.short	52230                   ; 0xcc06
	.short	50190                   ; 0xc40e
	.short	48214                   ; 0xbc56
	.short	46450                   ; 0xb572
	.short	44595                   ; 0xae33
	.short	42493                   ; 0xa5fd
	.short	40525                   ; 0x9e4d
	.short	38872                   ; 0x97d8
	.short	37002                   ; 0x908a
	.short	34636                   ; 0x874c
	.short	32769                   ; 0x8001
	.short	32798                   ; 0x801e
	.short	34810                   ; 0x87fa
	.short	37537                   ; 0x92a1
	.short	39945                   ; 0x9c09
	.short	42093                   ; 0xa46d
	.short	44405                   ; 0xad75
	.short	46860                   ; 0xb70c
	.short	49215                   ; 0xc03f
	.short	51476                   ; 0xc914
	.short	53802                   ; 0xd22a
	.short	56195                   ; 0xdb83
	.short	58550                   ; 0xe4b6
	.short	60850                   ; 0xedb2
	.short	63169                   ; 0xf6c1
	.short	0                       ; 0x0
	.short	2367                    ; 0x93f
	.short	4686                    ; 0x124e
	.short	6986                    ; 0x1b4a
	.short	9341                    ; 0x247d
	.short	11734                   ; 0x2dd6
	.short	14060                   ; 0x36ec
	.short	16321                   ; 0x3fc1
	.short	18676                   ; 0x48f4
	.short	21131                   ; 0x528b
	.short	23443                   ; 0x5b93
	.short	25591                   ; 0x63f7
	.short	27999                   ; 0x6d5f
	.short	30726                   ; 0x7806
	.short	32737                   ; 0x7fe1
	.short	32767                   ; 0x7fff
	.short	31072                   ; 0x7960
	.short	29199                   ; 0x720f
	.short	26952                   ; 0x6948
	.short	24717                   ; 0x608d
	.short	22699                   ; 0x58ab
	.short	20897                   ; 0x51a1
	.short	19177                   ; 0x4ae9
	.short	17387                   ; 0x43eb
	.short	15457                   ; 0x3c61
	.short	13424                   ; 0x3470
	.short	11388                   ; 0x2c7c
	.short	9437                    ; 0x24dd
	.short	7589                    ; 0x1da5
	.short	5785                    ; 0x1699
	.short	3941                    ; 0xf65
	.short	2007                    ; 0x7d7
	.short	0                       ; 0x0
	.short	63529                   ; 0xf829
	.short	61595                   ; 0xf09b
	.short	59751                   ; 0xe967
	.short	57947                   ; 0xe25b
	.short	56099                   ; 0xdb23
	.short	54148                   ; 0xd384
	.short	52112                   ; 0xcb90
	.short	50079                   ; 0xc39f
	.short	48149                   ; 0xbc15
	.short	46359                   ; 0xb517
	.short	44639                   ; 0xae5f
	.short	42837                   ; 0xa755
	.short	40819                   ; 0x9f73
	.short	38584                   ; 0x96b8
	.short	36337                   ; 0x8df1
	.short	34464                   ; 0x86a0
	.short	33399                   ; 0x8277
	.short	33454                   ; 0x82ae
	.short	34686                   ; 0x877e
	.short	36860                   ; 0x8ffc
	.short	39550                   ; 0x9a7e
	.short	42310                   ; 0xa546
	.short	44842                   ; 0xaf2a
	.short	47080                   ; 0xb7e8
	.short	49156                   ; 0xc004
	.short	51276                   ; 0xc84c
	.short	53582                   ; 0xd14e
	.short	56075                   ; 0xdb0b
	.short	58631                   ; 0xe507
	.short	61094                   ; 0xeea6
	.short	63382                   ; 0xf796
	.short	0                       ; 0x0
	.short	2154                    ; 0x86a
	.short	4442                    ; 0x115a
	.short	6905                    ; 0x1af9
	.short	9461                    ; 0x24f5
	.short	11954                   ; 0x2eb2
	.short	14260                   ; 0x37b4
	.short	16380                   ; 0x3ffc
	.short	18456                   ; 0x4818
	.short	20694                   ; 0x50d6
	.short	23226                   ; 0x5aba
	.short	25986                   ; 0x6582
	.short	28676                   ; 0x7004
	.short	30850                   ; 0x7882
	.short	32082                   ; 0x7d52
	.short	32137                   ; 0x7d89
	.short	30010                   ; 0x753a
	.short	28951                   ; 0x7117
	.short	27479                   ; 0x6b57
	.short	25695                   ; 0x645f
	.short	23700                   ; 0x5c94
	.short	21586                   ; 0x5452
	.short	19429                   ; 0x4be5
	.short	17287                   ; 0x4387
	.short	15194                   ; 0x3b5a
	.short	13165                   ; 0x336d
	.short	11199                   ; 0x2bbf
	.short	9285                    ; 0x2445
	.short	7406                    ; 0x1cee
	.short	5548                    ; 0x15ac
	.short	3698                    ; 0xe72
	.short	1849                    ; 0x739
	.short	0                       ; 0x0
	.short	63687                   ; 0xf8c7
	.short	61838                   ; 0xf18e
	.short	59988                   ; 0xea54
	.short	58130                   ; 0xe312
	.short	56251                   ; 0xdbbb
	.short	54337                   ; 0xd441
	.short	52371                   ; 0xcc93
	.short	50342                   ; 0xc4a6
	.short	48249                   ; 0xbc79
	.short	46107                   ; 0xb41b
	.short	43950                   ; 0xabae
	.short	41836                   ; 0xa36c
	.short	39841                   ; 0x9ba1
	.short	38057                   ; 0x94a9
	.short	36585                   ; 0x8ee9
	.short	35526                   ; 0x8ac6
	.short	34966                   ; 0x8896
	.short	34969                   ; 0x8899
	.short	35569                   ; 0x8af1
	.short	36761                   ; 0x8f99
	.short	38500                   ; 0x9664
	.short	40706                   ; 0x9f02
	.short	43271                   ; 0xa907
	.short	46067                   ; 0xb3f3
	.short	48964                   ; 0xbf44
	.short	51839                   ; 0xca7f
	.short	54593                   ; 0xd541
	.short	57160                   ; 0xdf48
	.short	59509                   ; 0xe875
	.short	61653                   ; 0xf0d5
	.short	63638                   ; 0xf896
	.short	0                       ; 0x0
	.short	1898                    ; 0x76a
	.short	3883                    ; 0xf2b
	.short	6027                    ; 0x178b
	.short	8376                    ; 0x20b8
	.short	10943                   ; 0x2abf
	.short	13697                   ; 0x3581
	.short	16572                   ; 0x40bc
	.short	19469                   ; 0x4c0d
	.short	22265                   ; 0x56f9
	.short	24830                   ; 0x60fe
	.short	27036                   ; 0x699c
	.short	28775                   ; 0x7067
	.short	29967                   ; 0x750f
	.short	30567                   ; 0x7767
	.short	30570                   ; 0x776a
	.short	27260                   ; 0x6a7c
	.short	26712                   ; 0x6858
	.short	25918                   ; 0x653e
	.short	24899                   ; 0x6143
	.short	23674                   ; 0x5c7a
	.short	22264                   ; 0x56f8
	.short	20691                   ; 0x50d3
	.short	18976                   ; 0x4a20
	.short	17139                   ; 0x42f3
	.short	15197                   ; 0x3b5d
	.short	13170                   ; 0x3372
	.short	11073                   ; 0x2b41
	.short	8921                    ; 0x22d9
	.short	6726                    ; 0x1a46
	.short	4500                    ; 0x1194
	.short	2255                    ; 0x8cf
	.short	0                       ; 0x0
	.short	63281                   ; 0xf731
	.short	61036                   ; 0xee6c
	.short	58810                   ; 0xe5ba
	.short	56615                   ; 0xdd27
	.short	54463                   ; 0xd4bf
	.short	52366                   ; 0xcc8e
	.short	50339                   ; 0xc4a3
	.short	48397                   ; 0xbd0d
	.short	46560                   ; 0xb5e0
	.short	44845                   ; 0xaf2d
	.short	43272                   ; 0xa908
	.short	41862                   ; 0xa386
	.short	40637                   ; 0x9ebd
	.short	39617                   ; 0x9ac1
	.short	38824                   ; 0x97a8
	.short	38276                   ; 0x9584
	.short	37990                   ; 0x9466
	.short	37982                   ; 0x945e
	.short	38262                   ; 0x9576
	.short	38839                   ; 0x97b7
	.short	39717                   ; 0x9b25
	.short	40895                   ; 0x9fbf
	.short	42367                   ; 0xa57f
	.short	44123                   ; 0xac5b
	.short	46146                   ; 0xb442
	.short	48416                   ; 0xbd20
	.short	50908                   ; 0xc6dc
	.short	53593                   ; 0xd159
	.short	56435                   ; 0xdc73
	.short	59400                   ; 0xe808
	.short	62447                   ; 0xf3ef
	.short	0                       ; 0x0
	.short	3089                    ; 0xc11
	.short	6136                    ; 0x17f8
	.short	9101                    ; 0x238d
	.short	11943                   ; 0x2ea7
	.short	14628                   ; 0x3924
	.short	17120                   ; 0x42e0
	.short	19390                   ; 0x4bbe
	.short	21413                   ; 0x53a5
	.short	23169                   ; 0x5a81
	.short	24641                   ; 0x6041
	.short	25819                   ; 0x64db
	.short	26697                   ; 0x6849
	.short	27274                   ; 0x6a8a
	.short	27554                   ; 0x6ba2
	.short	27546                   ; 0x6b9a
	.short	27260                   ; 0x6a7c
	.short	27129                   ; 0x69f9
	.short	26736                   ; 0x6870
	.short	26086                   ; 0x65e6
	.short	25185                   ; 0x6261
	.short	24041                   ; 0x5de9
	.short	22666                   ; 0x588a
	.short	21072                   ; 0x5250
	.short	19276                   ; 0x4b4c
	.short	17294                   ; 0x438e
	.short	15145                   ; 0x3b29
	.short	12850                   ; 0x3232
	.short	10432                   ; 0x28c0
	.short	7913                    ; 0x1ee9
	.short	5318                    ; 0x14c6
	.short	2672                    ; 0xa70
	.short	0                       ; 0x0
	.short	62864                   ; 0xf590
	.short	60218                   ; 0xeb3a
	.short	57623                   ; 0xe117
	.short	55104                   ; 0xd740
	.short	52686                   ; 0xcdce
	.short	50391                   ; 0xc4d7
	.short	48242                   ; 0xbc72
	.short	46260                   ; 0xb4b4
	.short	44464                   ; 0xadb0
	.short	42870                   ; 0xa776
	.short	41495                   ; 0xa217
	.short	40351                   ; 0x9d9f
	.short	39450                   ; 0x9a1a
	.short	38800                   ; 0x9790
	.short	38407                   ; 0x9607
	.short	38276                   ; 0x9584
	.short	38407                   ; 0x9607
	.short	38800                   ; 0x9790
	.short	39450                   ; 0x9a1a
	.short	40351                   ; 0x9d9f
	.short	41495                   ; 0xa217
	.short	42870                   ; 0xa776
	.short	44464                   ; 0xadb0
	.short	46260                   ; 0xb4b4
	.short	48242                   ; 0xbc72
	.short	50391                   ; 0xc4d7
	.short	52686                   ; 0xcdce
	.short	55104                   ; 0xd740
	.short	57623                   ; 0xe117
	.short	60218                   ; 0xeb3a
	.short	62864                   ; 0xf590
	.short	0                       ; 0x0
	.short	2672                    ; 0xa70
	.short	5318                    ; 0x14c6
	.short	7913                    ; 0x1ee9
	.short	10432                   ; 0x28c0
	.short	12850                   ; 0x3232
	.short	15145                   ; 0x3b29
	.short	17294                   ; 0x438e
	.short	19276                   ; 0x4b4c
	.short	21072                   ; 0x5250
	.short	22666                   ; 0x588a
	.short	24041                   ; 0x5de9
	.short	25185                   ; 0x6261
	.short	26086                   ; 0x65e6
	.short	26736                   ; 0x6870
	.short	27129                   ; 0x69f9
	.size	TBL, 1152

	.type	TBL_IDX,@object         ; @TBL_IDX
TBL_IDX:
	.ascii	"\000\000\000\000\001\002\003"
	.ascii	"\004\004\004\005\006\007\b"
	.size	TBL_IDX, 14

	.type	TBL_GAIN,@object        ; @TBL_GAIN
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	2
TBL_GAIN:
	.long	939537336               ; float 3.0565745E-5
	.long	941227067               ; float 3.67129505E-5
	.size	TBL_GAIN, 8


