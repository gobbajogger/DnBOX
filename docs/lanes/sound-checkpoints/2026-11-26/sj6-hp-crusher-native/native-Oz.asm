
leaf.Oz.o:	file format ELF32-pi32v2

Disassembly of section .text.sj6_hp10_44100:
sj6_hp10_44100:
       0:    76 04             	[--sp] = {rets, r6-r4}
       2:    50 ec 00 20       	r3_r2 = d[r0+0]
       6:    3f f5              <unknown instruction>
       8:    11 62             	r1 = [r1+8] 
       a:    04 62             	r4 = [r0+8] 
       c:    c5 ff 38 21 08 b6 	r5 = 3053986104 <sj6_hp10_44100+0xB6082138 : b6082138 >
      12:    3f e5              <unknown instruction>
      14:    61 23             	r1 = 131
      16:    c3 ff 50 d1 7f 3f 	r3 = 1065341264 <sj6_hp10_44100+0x3F7FD150 : 3f7fd150 >
      1c:    3f f5              <unknown instruction>
      1e:    22 23             	r2 = [sp+140]
      20:    03 63             	r3 = [r0+12] 
      22:    3f e5              <unknown instruction>
      24:    42 55             	if (r2 == 0) goto -214 <sj6_hp10_44100+0xFFFFFFFFFFFFFF50 : ffffffffffffff50 >
      26:    3f e5              <unknown instruction>
      28:    20 25             	r0 = [sp+148]
      2a:    c5 ff b2 a2 7f 3f 	r5 = 1065329330 <sj6_hp10_44100+0x3F7FA2B2 : 3f7fa2b2 >
      30:    3f e5              <unknown instruction>
      32:    32 35             	r2 |= 2097152
      34:    3f e5              <unknown instruction>
      36:    20 33             	r0 = [sp+204]
      38:    3f e5              <unknown instruction>
      3a:    40 23             	r0 = 3
      3c:    82 62             	[r0+8] = r2
      3e:    20 eb 42 00       	[r0+] = {r6, r1}
      42:    20 d6             	r0 = r2  #
      44:    83 63             		 [r0+12] = r3
      46:    56 04             	{pc, r6-r4} = [sp++]

Disassembly of section .text.sj6_zero_crusher_44100:
sj6_zero_crusher_44100:
       0:    74 04             	[--sp] = {rets, r4}
       2:    02 65             	r2 = [r0+20] 
       4:    03 64             	r3 = [r0+16] 
       6:    c4 ff 52 b8 7e 3f 	r4 = 1065269330 <sj6_zero_crusher_44100+0x3F7EB852 : 3f7eb852 >
       c:    3f f5              <unknown instruction>
       e:    22 24             	r2 = [sp+144]
      10:    81 64             	[r0+16] = r1
      12:    3f e5              <unknown instruction>
      14:    11 13             	r1 = b[r1++=r14] (u)
      16:    3f e5              <unknown instruction>
      18:    10 12             	r0 = b[r1++=r12] (u)
      1a:    10 d6             	r0 = r1  #
      1c:    81 65             		 [r0+20] = r1
      1e:    54 04             	{pc, r4} = [sp++]

Disassembly of section .text.sj6_hp_crusher_44100_span:
sj6_hp_crusher_44100_span:
       0:    78 04             	[--sp] = {rets, r8-r4}
       2:    34 16             	r4 = r3
       4:    25 16             	r5 = r2
       6:    16 16             	r6 = r1
       8:    07 16             	r7 = r0
       a:    08 e1 18 70       	r8 = r7 + 24
       e:    04 93             	goto 38 <sj6_hp_crusher_44100_span+0x36 : 36 >
      10:    70 d6             	r0 = r7  #
      12:    61 60             		 r1 = [r6+0] 
      14:    80 ea 00 00       	call 0 <sj6_hp_crusher_44100_span+0x18 : 18 >
      18:    01 16             	r1 = r0
      1a:    70 16             	r0 = r7
      1c:    80 ea 00 00       	call 0 <sj6_hp_crusher_44100_span+0x20 : 20 >
      20:    80 d6             	r0 = r8  #
      22:    e0 05             		 [r6++=4] = r0
      24:    51 60             	r1 = [r5+0] 
      26:    80 ea 00 00       	call 0 <sj6_hp_crusher_44100_span+0x2A : 2a >
      2a:    01 16             	r1 = r0
      2c:    80 16             	r0 = r8
      2e:    80 ea 00 00       	call 0 <sj6_hp_crusher_44100_span+0x32 : 32 >
      32:    d0 05             	[r5++=4] = r0
      34:    fc 3f             	r4 += -1
      36:    f4 4c             	if (r4 != 0) goto -40 <sj6_hp_crusher_44100_span+0x10 : 10 >
      38:    58 04             	{pc, r8-r4} = [sp++]

