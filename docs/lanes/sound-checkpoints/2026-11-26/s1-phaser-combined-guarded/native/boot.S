.section .text.crt0,"ax",@progbits
.globl _start
_start:
 goto 1f
 rts
1:
 cli
 sp = 0x01C7B000
 ssp = 0x01C7A000
 call bench
 goto .
