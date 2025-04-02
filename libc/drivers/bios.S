/* BIOS interrupt handling in assembly */
.section .text
.global bios_int
.global _bios_int
.type bios_int, @function
.type _bios_int, @function

/*
 * void bios_int(int interrupt_num, regs16_t* regs);
 * Call a BIOS interrupt with the specified registers
 */
bios_int:
_bios_int:
    // Save registers
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    
    // Get function arguments
    movl 8(%ebp), %eax     // interrupt_num
    movl 12(%ebp), %ebx    // regs pointer
    
    // Copy registers from the struct to actual registers
    movw 0(%ebx), %di      // di
    movw 2(%ebx), %si      // si
    movw 4(%ebx), %bp      // bp
    movw 6(%ebx), %dx      // dx
    movw 8(%ebx), %cx      // cx
    movw 10(%ebx), %bx     // bx
    movw 12(%ebx), %ax     // ax
    
    // Set up the interrupt
    movb %al, intnum
    
    // Switch to 16-bit mode and call the interrupt
    // We use a 16-bit code segment for this
    pushl $0                // Return offset
    pushw $0x0020           // Return segment (kernel code)
    pushw $0                // Flags
    pushw $0x0010           // Protected mode segment
    pushl $intr_16          // Offset to 16-bit code
    lret                    // Long return jumps to 16-bit code segment
    
intr_16:
    // Now in 16-bit mode
    // Set up real-mode-compatible segment registers
    movw $0, %ax
    movw %ax, %ds
    movw %ax, %es
    
    // Issue the interrupt
    .byte 0xCD              // INT instruction
intnum:
    .byte 0                 // Interrupt number filled in at runtime
    
    // Switch back to 32-bit protected mode
    ljmpl $0x0008, $intr_return_32 // Jump to 32-bit code segment
    
intr_return_32:
    // Now back in 32-bit mode
    
    // Get the regs pointer again (may have been clobbered)
    movl 12(%ebp), %ebx
    
    // Copy registers back to the struct
    movw %di, 0(%ebx)      // di
    movw %si, 2(%ebx)      // si
    movw %bp, 4(%ebx)      // bp
    movw %dx, 6(%ebx)      // dx
    movw %cx, 8(%ebx)      // cx
    movw %bx, 10(%ebx)     // bx
    movw %ax, 12(%ebx)     // ax
    
    // Restore registers and return
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret
