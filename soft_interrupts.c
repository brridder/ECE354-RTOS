#include "soft_interrupts.h"

#include "kernel.h"
#include "dbug.h"

/**
 * @brief: Software interrupt handler used to make system calls.
 *
 * This is installed in vector 0. The system call to make is passed in D0.
 * Return value is in D0. Arguments are passed in D1-D3.
 * 
 * Supported system calls:
 *   0: release_processor()
 *   1: get_process_priority(int pid)
 *           
 */

void system_call() {
    int call_id;
    int args[3];
    int* stack_iter;
    int return_value;

    // 
    // In supervisor mode. Disable interrupts.
    //

    asm("move.w #0x2700, %sr");

    //
    // Get call ID and arguments.
    // 

    asm("move.l %%d0, %0" : "=r" (call_id));
    asm("move.l %%d1, %0" : "=r" (args[0]));
    asm("move.l %%d2, %0" : "=r" (args[1]));
    asm("move.l %%d3, %0" : "=r" (args[2]));

    //
    // Save current user process state (register contents), as well as 
    // exception frame. It is necessary to save state at this point
    // because this system call could potentially cause a context switch.
    //

    stack_iter = (int*)running_process->stack;
    asm("move.l (8, %%sp), %0" : "=r" (*(--stack_iter))); // PC
    asm("move.l (4, %%sp), %0" : "=r" (*(--stack_iter))); // SR

    /*
    asm("move.l %%a0, %0" : "=r" (*(--stack_iter))); // A0
    asm("move.l %%a1, %0" : "=r" (*(--stack_iter))); // A1
    asm("move.l %%a2, %0" : "=r" (*(--stack_iter))); // A2
    asm("move.l %%a3, %0" : "=r" (*(--stack_iter))); // A3
    asm("move.l %%a4, %0" : "=r" (*(--stack_iter))); // A4
    asm("move.l %%a5, %0" : "=r" (*(--stack_iter))); // A5
    asm("move.l %%a6, %0" : "=r" (*(--stack_iter))); // A6
    asm("move.l %%d0, %0" : "=r" (*(--stack_iter))); // D0
    asm("move.l %%d1, %0" : "=r" (*(--stack_iter))); // D1
    asm("move.l %%d2, %0" : "=r" (*(--stack_iter))); // D2
    asm("move.l %%d3, %0" : "=r" (*(--stack_iter))); // D3
    asm("move.l %%d4, %0" : "=r" (*(--stack_iter))); // D4
    asm("move.l %%d5, %0" : "=r" (*(--stack_iter))); // D5
    asm("move.l %%d6, %0" : "=r" (*(--stack_iter))); // D6
    asm("move.l %%d7, %0" : "=r" (*(--stack_iter))); // D7
    */

    running_process->stack = (void*)stack_iter;
    running_process->state = STATE_READY;
    

    return_value = -1;
    switch(call_id) {

        //
        // 0: release_processor()
        // 

        case 0:
            return_value = k_release_processor(); 
            break;

        //
        // 1: get_process_priority(int pid)
        //
            
        case 1:
            return_value = k_get_process_priority(args[0]);
            break;    

        //
        // Invalid call ID
        //

        default:
            // TODO: Handle this case
            rtx_dbug_outs("Error: Invalid system call ID\r\n");
            break;
    }

    //
    // Restore user process registers and exception frame. 
    // At this point, we know that the running process is the one that 
    // initiated the system call because a process change would have already
    // `rte`ed, and the following code would not run.
    //

    /*
    asm("move.l %0, %%d7" : : "r" (*(stack_iter++)) : "%%d7"); // D7
    asm("move.l %0, %%d6" : : "r" (*(stack_iter++)) : "%%d6"); // D6
    asm("move.l %0, %%d5" : : "r" (*(stack_iter++)) : "%%d5"); // D5
    asm("move.l %0, %%d4" : : "r" (*(stack_iter++)) : "%%d4"); // D4
    asm("move.l %0, %%d3" : : "r" (*(stack_iter++)) : "%%d4"); // D3
    asm("move.l %0, %%d2" : : "r" (*(stack_iter++)) : "%%d4"); // D2
    asm("move.l %0, %%d1" : : "r" (*(stack_iter++)) : "%%d4"); // D1
    asm("move.l %0, %%d0" : : "r" (*(stack_iter++)) : "%%d4"); // D0
    asm("move.l %0, %%a6" : : "r" (*(stack_iter++)) : "%%a6"); // A6
    asm("move.l %0, %%a5" : : "r" (*(stack_iter++)) : "%%a5"); // A5
    asm("move.l %0, %%a4" : : "r" (*(stack_iter++)) : "%%a4"); // A4
    asm("move.l %0, %%a3" : : "r" (*(stack_iter++)) : "%%a3"); // A3
    asm("move.l %0, %%a2" : : "r" (*(stack_iter++)) : "%%a2"); // A2
    asm("move.l %0, %%a1" : : "r" (*(stack_iter++)) : "%%a1"); // A1
    asm("move.l %0, %%a0" : : "r" (*(stack_iter++)) : "%%a0"); // A0
    */

    asm("move.l %0, %%sp" : : "r" (stack_iter) : "%%sp"); //SP
    stack_iter = stack_iter + 2;

    //
    // Save the user process' stack pointer. We add 8 bytes to skip past the
    // exception frame which will be removed by the following `rte` instruction
    //    

    running_process->stack = (void*)stack_iter;
    running_process->state = STATE_RUNNING;
	
    //
    // Return from the exception. We have to unlink the 
    // frame pointer manually because GCC 
    // doesn't know we are exiting early.
    //

    asm("move.l %0, %%d0" : : "r" (return_value));

    asm("unlk %fp");
    asm("rte");
}
