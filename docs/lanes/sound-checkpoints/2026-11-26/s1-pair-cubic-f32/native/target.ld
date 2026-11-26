ENTRY(_start)
MEMORY {
 XIP (rx) : ORIGIN = 0x02000120, LENGTH = 0x10000
 RAM (rw) : ORIGIN = 0x01C09000, LENGTH = 0x10000
}
SECTIONS {
 .text : { KEEP(*(.text.crt0)) *(.text*) *(.rodata*) . = ALIGN(4); } > XIP
 .bss (NOLOAD) : { *(.bss*) *(COMMON) . = ALIGN(4); } > RAM
 /DISCARD/ : { *(.comment) *(.eh_frame*) }
}
