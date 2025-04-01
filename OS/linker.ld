OUTPUT_FILE_CONTENTS
SECTIONS
{
    . = 0x100000; /* Start of the kernel code */
    
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