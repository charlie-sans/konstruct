/* Linker script for OS kernel */
SECTIONS
{
    . = 0x100000; /* Start of the kernel code at 1MB mark to match boot.asm */
    
    .text : {
        *(.text)          /* All .text sections from input files */
    }
    
    .data : {
        *(.data)          /* All .data sections from input files */
    }
    
    .bss : {
        *(.bss)           /* All .bss sections from input files */
        *(COMMON)         /* Common symbols */
    }
    
    /DISCARD/ : {
        *(.comment)       /* Discard comments */
        *(.note)          /* Discard notes */
    }
}