MEMORY {
 APP (rx) : ORIGIN = 0x02000120, LENGTH = 552672
 RAM (rw) : ORIGIN = 0x01c08000, LENGTH = 96K
}
SECTIONS {
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > APP
 .data : { *(.data*) . = ALIGN(4); } > RAM AT > APP
 .bss (NOLOAD) : { *(.bss*) *(COMMON) . = ALIGN(4); } > RAM
 /DISCARD/ : { *(.comment) *(.note*) }
}
