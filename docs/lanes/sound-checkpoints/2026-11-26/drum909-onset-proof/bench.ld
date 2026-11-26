ENTRY(_start)
SECTIONS {
  . = 0x02000120;
  .text : { KEEP(*(.text.crt0)) *(.text*) *(.rodata*) }
  .bss 0x01c04000 (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.eh_frame*) *(.comment*) }
}
