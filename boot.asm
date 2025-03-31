; Simple bootloader
[bits 16]
[org 0x7c00]

; Set up stack
mov bp, 0x9000
mov sp, bp

; Store boot drive number that BIOS provides in DL
mov [BOOT_DRIVE], dl

; Print a message
mov si, MSG_REAL_MODE
call print_string

; Load the kernel
call load_kernel

; Print success message
mov si, KERNEL_LOADED_MSG
call print_string

; Switch to protected mode
call switch_to_pm

; Print string function
print_string:
    ; Function implementation to print a null-terminated string
    lodsb
    or al, al
    jz done
    mov ah, 0x0e
    int 0x10
    jmp print_string
done:
    ret

; Load kernel function
load_kernel:
    ; Code to load kernel from disk to memory
    mov si, LOAD_KERNEL_MSG
    call print_string
    
    mov bx, KERNEL_OFFSET  ; Destination address
    mov dh, 15             ; Number of sectors to read
    mov dl, [BOOT_DRIVE]   ; Drive to read from
    call disk_load
    ret

; Add disk_load function
disk_load:
    push dx         ; Store DX on stack to check against total sectors read
    
    mov ah, 0x02    ; BIOS read sector function
    mov al, dh      ; Read DH sectors
    mov ch, 0x00    ; Select cylinder 0
    mov dh, 0x00    ; Select head 0
    mov cl, 0x02    ; Start reading from second sector (after boot sector)
    
    int 0x13        ; BIOS interrupt
    
    jc disk_error   ; Jump if error (carry flag set)
    
    pop dx          ; Restore DX from the stack
    cmp dh, al      ; If AL (sectors read) != DH (sectors expected)
    jne sectors_error  ; Display error
    ret
    
disk_error:
    mov si, DISK_ERROR_MSG
    call print_string
    
    ; Print error code
    mov ah, 0
    mov al, ah      ; Error code in AH after int 0x13
    add al, '0'     ; Convert to ASCII
    mov ah, 0x0e
    int 0x10
    
    jmp $           ; Hang

sectors_error:
    mov si, SECTORS_ERROR_MSG
    call print_string
    jmp $           ; Hang

; GDT
gdt_start:

gdt_null:           ; Null descriptor - required
    dd 0x0          ; Double word (4 bytes) of zeros
    dd 0x0

gdt_code:           ; Code segment descriptor
    ; Base=0x0, Limit=0xfffff
    ; 1st flags: (present)1 (privilege)00 (descriptor type)1 -> 1001b
    ; Type flags: (code)1 (conforming)0 (readable)1 (accessed)0 -> 1010b
    ; 2nd flags: (granularity)1 (32-bit default)1 (64-bit seg)0 (AVL)0 -> 1100b
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_data:           ; Data segment descriptor
    ; Same as code segment except for type flags
    ; Type flags: (code)0 (expand down)0 (writable)1 (accessed)0 -> 0010b
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT, one less than true size
    dd gdt_start                ; Address of GDT

; Segment selector constants
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Switch to protected mode function
switch_to_pm:
    cli                     ; Disable interrupts
    lgdt [gdt_descriptor]   ; Load GDT
    
    ; Set PE bit in CR0 to enter protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush pipeline and set CS
    jmp CODE_SEG:init_pm

[bits 32]
; Initialization routine for protected mode
init_pm:
    ; Update segment registers
    mov ax, DATA_SEG        ; Point all segment registers to the data segment
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Update stack position
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Call some 32-bit code
    call BEGIN_PM

; 32-bit protected mode code
BEGIN_PM:
    ; Load the kernel into memory
    mov ebx, KERNEL_OFFSET  ; Point to the loaded kernel
    jmp ebx                 ; Jump to the kernel

; Constants and variables
MSG_REAL_MODE db "Started in 16-bit Real Mode", 13, 10, 0
DISK_ERROR_MSG db "Disk read error! Error code: ", 0
SECTORS_ERROR_MSG db "Incorrect number of sectors read!", 13, 10, 0
LOAD_KERNEL_MSG db "Loading kernel into memory...", 13, 10, 0
KERNEL_LOADED_MSG db "Kernel loaded successfully!", 13, 10, 0
BOOT_DRIVE db 0
KERNEL_OFFSET equ 0x1000

; Boot sector padding
times 510-($-$$) db 0
dw 0xaa55
