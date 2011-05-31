#include "soft_interrupts.h"

#include "kernel.h"
#include "dbug.h"

/**
 * @brief: Software interrupt handler used to make system calls.
 *
 * This is installed in vector 0. The system call to make is passed in D0. 
 * 
 * Supported system calls:
 *   0: release_processor()
 *           
 */

void system_call() {
    int call_id;

    // 
    // In supervisor mode. Disable interrupts.
    //

    asm("move.w #0x2700, %sr");

    //
    // Get call ID
    // 
    
    asm("move.l %d0, %0" : "=r" (call_id));
	
    asm("move.l %a0, -(%a7)");
    asm("move.l %a1, -(%a7)");
    asm("move.l %a2, -(%a7)");
    asm("move.l %a3, -(%a7)");
    asm("move.l %a4, -(%a7)");
    asm("move.l %a5, -(%a7)");
    asm("move.l %a6, -(%a7)");
    asm("move.l %d0, -(%a7)");
    asm("move.l %d1, -(%a7)");
    asm("move.l %d2, -(%a7)");
    asm("move.l %d3, -(%a7)");
    asm("move.l %d4, -(%a7)");
    asm("move.l %d5, -(%a7)");
    asm("move.l %d6, -(%a7)");
    asm("move.l %d7, -(%a7)");
	
    switch(call_id) {

        //
        // 0 : release_processor()
        // 

        case 0:
            k_release_processor(); 
            break;

        //
        // Invalid call ID
        //

        default:
            // TODO: Handle this case

            rtx_dbug_outs("Error: Invalid system call ID\r\n");

            break;
    }
   
    asm("move.l (%a7)+, %d7");
    asm("move.l (%a7)+, %d6");
    asm("move.l (%a7)+, %d5");
    asm("move.l (%a7)+, %d4");
    asm("move.l (%a7)+, %d3");
    asm("move.l (%a7)+, %d2");
    asm("move.l (%a7)+, %d1");
    asm("move.l (%a7)+, %d0");
    asm("move.l (%a7)+, %a6");
    asm("move.l (%a7)+, %a5");
    asm("move.l (%a7)+, %a4");
    asm("move.l (%a7)+, %a3");
    asm("move.l (%a7)+, %a2");
    asm("move.l (%a7)+, %a1");
    asm("move.l (%a7)+, %a0");
	
    //
    // Return from the exception. We have to unlink the 
    // frame pointer manually because GCC 
    // doesn't know we are exiting early.
    //

    asm("unlk %a6");
    asm("rte");
}
