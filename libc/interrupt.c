
#include <sys/types.h>
#include <interrupt.h>
#include <syscall.h>

// IDT table with 256 entries
static idt_entry_t idt_entries[256];
static idt_descriptor_t idt_desc;

// Initialize the IDT
void idt_init(void) {
    // Set up the IDT descriptor
    idt_desc.limit = sizeof(idt_entries) - 1;
    idt_desc.base = (uint32_t)&idt_entries;
    
    // Clear the IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Set up the syscall entry (interrupt 0x80)
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0x8E);
    
    // Load the IDT
    idt_load(&idt_desc);
}

// Set an entry in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].offset_low = (base & 0xFFFF);
    idt_entries[num].selector = selector;
    idt_entries[num].zero = 0;
    idt_entries[num].type_attr = flags;
    idt_entries[num].offset_high = ((base >> 16) & 0xFFFF);
}


void idt_load(idt_descriptor_t* idt_desc) {
    asm volatile("lidt (%0)" : : "r" (idt_desc));
}


void syscall_handler(void) {

    asm volatile(
        "pushl %%eax\n"
        "pushl %%ebx\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "pushl %%esi\n"
        "call handle_syscall\n"
        "addl $20, %%esp\n"
        "iret\n"
        :
        :
        : "memory"
    );
}
