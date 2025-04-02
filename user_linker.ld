ENTRY(main)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x400000; /* User programs start at 4MB */

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
