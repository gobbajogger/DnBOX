.section .text.crt0,"ax",@progbits
.globl _start
_start:
   cli
   sp = 0x01c18000
   ssp = 0x01c18400
   call bench_start
   goto .
