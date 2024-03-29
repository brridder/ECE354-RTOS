#include "rtx_inc.h"
#include "./lib/dbug.h"
#include "./core/init.h"
#include "./core/kernel.h"
#include "rtx.h"

extern void* __end;
extern void __REGISTER_TEST_PROCS_ENTRY__();

int __main(void) {
    return 0;
}

int main(void) {
    VOID* memory_start;
    
    //
    // Disable interrupts
    //

    asm("move.w #0x2700, %sr");

    memory_start = &__end;
    init(memory_start);

    //
    // Register the test processes. Required by test suite.
    //

    //__REGISTER_TEST_PROCS_ENTRY__();

    //
    // Begin running processes
    //

    k_release_processor();

    //
    // Enable interrupts
    // 

    asm("move.w #0x2000, %sr");

    return 0;
}
