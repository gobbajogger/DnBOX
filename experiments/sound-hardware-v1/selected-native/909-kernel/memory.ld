MEMORY {
 XIP (rx) : ORIGIN = 0x02090000, LENGTH = 0x70000
 RAM (rw) : ORIGIN = 0x01c90000, LENGTH = 96K
}
SECTIONS {
 .text : { KEEP(*(.text.crt0)) *(.text*) *(.rodata*) . = ALIGN(4); } > XIP
 .bss (NOLOAD) : { *(.bss*) *(COMMON) . = ALIGN(4); } > RAM
}

