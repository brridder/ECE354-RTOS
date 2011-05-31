#include "rtx_inc.h"
#include "dbug.h"
#include "memory.h"
#include "init.h"
#include "kernel.h"

extern void* __end;

int __main(void) {
    return 0;
}

int main(void) {
    VOID* stack_start;

    //
    // Disable interrupts
    //

    asm("move.w #0x2700, %sr");

    stack_start = &__end;

    // TODO :: Find top of stack.
    rtx_dbug_outs("Initializing processes...");
    init_processes(stack_start);
    rtx_dbug_outs("done.\r\n");

    rtx_dbug_outs("Initializing interrupts...");
    init_interrupts();
    rtx_dbug_outs("done.\r\n");

    //
    // Change to null process (PID 0)
    //

    rtx_dbug_outs("Switching to null process...\r\n");
    k_change_process(&processes[0]);

    //
    // Enable interrupts
    // 

    asm("move.w #0x2000, %sr");

    return 0;
}
