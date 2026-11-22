.section .text.crt0,"ax",@progbits
.globl _start
_start:
 cli
 sp = 0x1c7a000
 ssp = 0x1c7c000
 call bench
 r0 = 0
 goto r0
