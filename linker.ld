ENTRY(kernel_main)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x1000;
    
    /* For Linux/standard ELF systems */
    .text : {
        *(.text.start)  /* Include the .text.start section inside .text */
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
    }
}
