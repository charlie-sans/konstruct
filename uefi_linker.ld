ENTRY(efi_main)
OUTPUT_FORMAT(pei-x86-64)

SECTIONS
{
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .rodata : {
        *(.rodata)
    }
    
    .bss : {
        *(.bss)
    }
    
    /DISCARD/ : {
        *(.note.*)
        *(.eh_frame)
    }
}
