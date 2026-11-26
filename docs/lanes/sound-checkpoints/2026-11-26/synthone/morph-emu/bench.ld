ENTRY(_start)
SECTIONS {
  . = 0x02000120;
  .text : { KEEP(*(.text.crt0)) *(.text*) *(.rodata*) }
  /DISCARD/ : { *(.eh_frame*) *(.comment*) }
}
