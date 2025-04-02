#include "bios.h"

/* 
 * Wrapper function for bios_int in case the assembly function name
 * is being mangled or not exported correctly
 */
void bios_int(int interrupt_num, regs16_t* regs) {
    /* This is just a wrapper to ensure the function is visible to the linker */
    /* The actual implementation is in bios.S */
    
    /* This line will be replaced by the actual assembly implementation */
    __asm__ volatile (
        "call _bios_int\n"  // Call the assembly function with the proper name
        :
        : "a" (interrupt_num), "b" (regs)
        : "memory"
    );
}
