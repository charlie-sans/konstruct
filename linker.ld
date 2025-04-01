ENTRY(kernel_main)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x100000; /* Load the kernel at 1MB */

    .multiboot : {
        *(.multiboot) /* Include the multiboot header */
    }

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}
