#include "soft_interrupts.h"

#include "kernel.h"
#include "dbug.h"

#include "rtx.h"

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
    // Save current user process state (register contents),
    //
    
    asm("move.l %a0, -(%sp)"); // A0
    asm("move.l %a1, -(%sp)"); // A1
    asm("move.l %a2, -(%sp)"); // A2
    asm("move.l %a3, -(%sp)"); // A3
    asm("move.l %a4, -(%sp)"); // A4
    asm("move.l %a5, -(%sp)"); // A5
    asm("move.l %a6, -(%sp)"); // A6
    asm("move.l %d4, -(%sp)"); // D4
    asm("move.l %d5, -(%sp)"); // D5
    asm("move.l %d6, -(%sp)"); // D6
    asm("move.l %d7, -(%sp)"); // D7

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
    // Restore user process registers.
    //

    asm("move.l (%sp)+, %d7"); // D7
    asm("move.l (%sp)+, %d6"); // D6
    asm("move.l (%sp)+, %d5"); // D5
    asm("move.l (%sp)+, %d4"); // D4
    asm("move.l (%sp)+, %a6"); // A6
    asm("move.l (%sp)+, %a5"); // A5
    asm("move.l (%sp)+, %a4"); // A4
    asm("move.l (%sp)+, %a3"); // A3
    asm("move.l (%sp)+, %a2"); // A2
    asm("move.l (%sp)+, %a1"); // A1
    asm("move.l (%sp)+, %a0"); // A0

    //
    // Return from the exception. We have to unlink the 
    // frame pointer manually because GCC 
    // doesn't know we are exiting early.
    //

    asm("move.l %0, %%d0" : : "r" (return_value));

    asm("unlk %fp");
    asm("rte");
}
