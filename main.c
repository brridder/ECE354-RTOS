#include "rtx_inc.h"
#include "dbug.h"
#include "memory.h"
#include "init.h"
#include "kernel.h"
#include "rtx.h"
//#include "loader/rtx_test.h"

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
