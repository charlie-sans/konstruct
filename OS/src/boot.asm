; Simple bootloader
[bits 16]
[org 0x7c00]

; Start by jumping over the data section
jmp boot_start

; Constants and variables (keep these small)
BOOT_DRIVE db 0
MSG_LOADING db "Loading...", 13, 10, 0
LOAD_ERR_MSG db "Load error: ", 0
STAGE2_ADDR equ 0x7E00  ; Address where stage 2 will be loaded

boot_start:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax          ; Ensure ES is zero for proper memory addressing
    
    ; Set up stack
    mov bp, 0x9000
    mov sp, bp

    ; Store boot drive number that BIOS provides in DL
    mov [BOOT_DRIVE], dl

    ; Print loading message
    mov si, MSG_LOADING
    call print_string

    ; Reset disk system first to ensure it's in a known state
    xor ax, ax
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error

    ; Load stage 2 to memory
    mov bx, STAGE2_ADDR ; Destination address (ES:BX)
    mov ah, 0x02      ; BIOS read function
    mov al, 3         ; Read 3 sectors (larger second stage)
    mov ch, 0         ; Cylinder 0
    mov cl, 2         ; Sector 2 (right after boot sector)
    mov dh, 0         ; Head 0
    mov dl, [BOOT_DRIVE]  ; Drive number from BIOS
    int 0x13
    jc disk_error

    ; Print a period to show stage 2 was loaded
    mov ah, 0x0e
    mov al, '.'
    int 0x10

    ; Jump to stage 2
    jmp 0:STAGE2_ADDR   ; Far jump with explicit segment

; Error handler - keep it minimal
disk_error:
    mov si, LOAD_ERR_MSG
    call print_string
    
    ; Print error code
    mov al, ah          ; Error code is in AH
    add al, '0'         ; Convert to ASCII
    mov ah, 0x0e
    int 0x10
    
    jmp $               ; Hang

; Simple print string routine
print_string:
    lodsb               ; Load byte at DS:SI into AL
    or al, al           ; Test if character is zero (end of string)
    jz done
    mov ah, 0x0e        ; BIOS teletype function
    int 0x10            ; Call BIOS to print character
    jmp print_string
done:
    ret

; Boot sector padding
times 510-($-$$) db 0
dw 0xaa55

; ----------------------- STAGE 2 STARTS HERE -----------------------
; This code will be loaded at STAGE2_ADDR (0x7E00)
stage2_start:
    ; Make sure interrupts are disabled while we set up
    cli
    
    ; Set up our own segment registers again (don't trust what was passed)
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up a new stack to be safe
    mov bp, 0x9000
    mov sp, bp
    
    ; Restore interrupts
    sti
    
    ; Print stage 2 message - use direct BIOS calls to keep it simple
    mov si, MSG_STAGE2
    call print_string
    
    ; Try to print a simple character to check if we're still running
    mov ah, 0x0e
    mov al, '*'         ; Print a star as an alive indicator
    int 0x10

    ; Load the kernel
    call load_kernel
    
    ; Print success message
    mov si, KERNEL_LOADED_MSG
    call print_string

    ; Switch to protected mode
    call switch_to_pm

; Stage 2 data
MSG_STAGE2 db "Stage 2!", 13, 10, 0
MSG_REAL_MODE db "16-bit mode", 13, 10, 0
RESET_ERROR_MSG db "Reset error! Code: ", 0
SECTORS_ERROR_MSG db "Sector error!", 13, 10, 0
LOAD_KERNEL_MSG db "Loading kernel...", 13, 10, 0
KERNEL_LOADED_MSG db "Kernel loaded!", 13, 10, 0
HANG_MSG db "-System halted", 13, 10, 0
KERNEL_OFFSET equ 0x100000  ; Final location of kernel (1MB mark)

; Print hex value in DX - moved to stage 2
print_hex:
    pusha
    mov cx, 4       ; 4 hex digits (16 bits)
hex_loop:
    dec cx          ; Start from most significant digit
    mov ax, dx      ; Copy DX to AX
    shr ax, cl      ; Shift right to isolate current digit
    shr ax, cl
    shr ax, cl
    shr ax, cl
    and ax, 0xf     ; Mask out all but the lower 4 bits
    
    ; Convert to ASCII
    add al, '0'     ; Add '0' to convert to ASCII
    cmp al, '9'     ; If greater than '9', add additional 7 to get to 'A'-'F'
    jle hex_print
    add al, 7       ; 'A' is ASCII 65, '9' is 57, so 65-57 = 8, but we subtract 1
hex_print:
    mov ah, 0x0e    ; BIOS teletype function
    int 0x10        ; Print character in AL
    
    test cx, cx     ; Check if we're done
    jnz hex_loop
    
    popa
    ret

; Load kernel function - now in stage 2
load_kernel:
    ; Print message
    mov si, LOAD_KERNEL_MSG
    call print_string
    
    ; Reset disk system first
    xor ax, ax        ; AH = 0 (reset disk system)
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc reset_error
    
    ; We'll use a simpler approach to load the kernel
    ; Load directly to a lower address, we'll copy it later
    xor ax, ax
    mov es, ax        ; Ensure ES is 0
    mov bx, 0x8000    ; Temporary buffer at 0x8000
    
    ; Read kernel sectors
    mov ah, 0x02      ; BIOS read function
    mov al, 10        ; Read 10 sectors at a time (improved speed)
    mov ch, 0         ; Cylinder 0
    mov cl, 5         ; Sector 5 (after boot sector & stage 2)
    mov dh, 0         ; Head 0
    mov dl, [BOOT_DRIVE]
    int 0x13          ; BIOS interrupt
    jc disk_read_error
    
    ; Print a progress indicator
    mov ah, 0x0e
    mov al, '+'
    int 0x10
    
    ; Read remaining sectors if needed
    mov bx, 0x8000 + (10 * 512) ; Next buffer position
    mov ah, 0x02
    mov al, 10        ; Read another 10 sectors
    mov cl, 15        ; Start from sector 15
    int 0x13
    jc disk_read_error
    
    ; Print progress
    mov ah, 0x0e
    mov al, '+'
    int 0x10
    
    ; Read final batch of sectors
    mov bx, 0x8000 + (20 * 512) ; Next buffer position
    mov ah, 0x02
    mov al, 10        ; Read final 10 sectors
    mov cl, 25        ; Start from sector 25
    int 0x13
    jc disk_read_error
    
    ; Print progress completed
    mov ah, 0x0e
    mov al, '#'
    int 0x10
    
    ret

reset_error:
    mov si, RESET_ERROR_MSG
    call print_string
    mov al, ah        ; Error code in AH
    add al, '0'
    mov ah, 0x0e
    int 0x10
    jmp hang_system

disk_read_error:
    mov si, SECTORS_ERROR_MSG
    call print_string
    mov al, ah        ; Error code in AH
    add al, '0'
    mov ah, 0x0e
    int 0x10
    
hang_system:
    mov si, HANG_MSG
    call print_string
    jmp $             ; Infinite loop

; GDT - now in stage 2
gdt_start:
    ; Null descriptor
    dd 0x0, 0x0
    
    ; Code segment descriptor
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

    ; Data segment descriptor
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
CODE_SEG equ 8     ; 8 bytes from start of GDT
DATA_SEG equ 16    ; 16 bytes from start of GDT

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
    ; Debug - mark in video memory that we reached protected mode
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0A  ; Green on black
    
    ; Copy kernel from temporary location to final destination
    ; Source: 0x8000, Destination: 0x100000, Size: ~60KB (30 sectors)
    mov esi, 0x8000         ; Source (after stage 2)
    mov edi, 0x100000       ; Destination - kernel expects to be at 1MB mark
    mov ecx, 3840           ; 30 sectors * 512 bytes / 4 = 3840 dwords
    cld                     ; Make sure we're moving forward
    rep movsd               ; Copy ECX dwords from ESI to EDI
    
    ; Debug - mark in video memory that kernel was copied
    mov byte [0xB8002], 'K'
    mov byte [0xB8003], 0x0A  ; Green on black
    
    ; Jump to kernel at its expected address
    mov ebx, 0x100000       ; Kernel entry point at 1MB
    call ebx                ; Call the kernel
    
    ; If kernel returns, show indicator and hang
    mov byte [0xB8004], 'R'  ; 'R' for Returned
    mov byte [0xB8005], 0x04 ; Red on black
    
    jmp $                   ; Hang if we ever return from kernel

; End of stage 2
