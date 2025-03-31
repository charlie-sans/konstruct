ENTRY(kernel_main)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x1000;
    
    .text.start : {
        *(.text.start)
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
    }
}
