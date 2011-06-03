#include "kernel.h"

#include "dbug.h"
#include "process.h"
#include "soft_interrupts.h"

/**
 * @brief: System call used by a running proccess to release the processor.
 */
int k_release_processor() {
    rtx_dbug_outs("k_release_processor()\r\n");
    
    //
    // TODO: Make a decision on what process to run
    //

    //
    // TODO: Right now we simple toggle between two dummy processes
    // for testing purposes.
    // 

    if (running_process == &processes[0]) {
        k_change_process(&processes[1]);
    } else {
        k_change_process(&processes[0]);
    }

    return 0;
}

/**
 * @brief: Returns the priority of a process
 * @param: pid the pid of the process
 */
int k_get_process_priority(int pid) {
    rtx_dbug_outs("k_get_process_priority()\r\n");

    //
    // TODO: Error handling
    //

    return processes[pid].priority;
}

/**
 * @brief: Performs a context switch. After the context switch, the 
 *         process begins executing in user mode.
 * @param: process the process to switch to.
 */
void k_change_process(process_control_block* process) {
    process_control_block* previous_process;

    rtx_dbug_outs("k_change_process()\r\n");

    if (!process) {
        rtx_dbug_outs("  Invalid process handle\r\n");
        return;
    }

    // 
    // Get current running process. If there is a process currently running,
    // perform a context switch.
    // 

    previous_process = running_process;
    if (previous_process) {

        //
        // If the process has not saved it's state, do it now.
        //

        if (previous_process->state == STATE_RUNNING) {
            rtx_dbug_outs("  Saving process state\r\n");
      
            //
            // Save register contents. The exception frame is already on the 
            // stack.
            //

            asm("move.l %a0, -(%sp)"); // A0
            asm("move.l %a1, -(%sp)"); // A1
            asm("move.l %a2, -(%sp)"); // A2
            asm("move.l %a3, -(%sp)"); // A3
            asm("move.l %a4, -(%sp)"); // A4
            asm("move.l %a5, -(%sp)"); // A5
            asm("move.l %a6, -(%sp)"); // A6
            asm("move.l %d0, -(%sp)"); // D0
            asm("move.l %d1, -(%sp)"); // D1
            asm("move.l %d2, -(%sp)"); // D2
            asm("move.l %d3, -(%sp)"); // D3
            asm("move.l %d4, -(%sp)"); // D4
            asm("move.l %d5, -(%sp)"); // D5
            asm("move.l %d6, -(%sp)"); // D6
            asm("move.l %d7, -(%sp)"); // D7
            
            asm("move.l %%sp, %0": "=r" (previous_process->stack));

            previous_process->state = STATE_READY;
        }
    }    

    //
    // Switch to the process
    //

    running_process = process;
    asm("move.l %0, %%sp" : : "r" (running_process->stack) : "%%sp");

    if (running_process->state == STATE_STOPPED) {
        rtx_dbug_outs("  Starting process\r\n");

        running_process->state = STATE_RUNNING;

        //
        // If the process has not been started yet, we need to start it by
        // calling `rte`. The exception frame generated in init_processes is 
        // the only thing on its stack.
        //

        asm("rte");
    } else if (running_process->state == STATE_READY) {
        rtx_dbug_outs("  Loading process state\r\n");

        running_process->state = STATE_RUNNING;

        //
        // This process has already been started and has had its state saved 
        // on its stack. Restore the registers, and return. Whatever caused this
        // process to be switched out in the first place will end up calling
        // `rte`.
        //
    
        asm("move.l (%sp)+, %d7"); // D7
        asm("move.l (%sp)+, %d6"); // D6
        asm("move.l (%sp)+, %d5"); // D5
        asm("move.l (%sp)+, %d4"); // D4
        asm("move.l (%sp)+, %d3"); // D3
        asm("move.l (%sp)+, %d2"); // D2
        asm("move.l (%sp)+, %d1"); // D1
        asm("move.l (%sp)+, %d0"); // D0
        asm("move.l (%sp)+, %a6"); // A6
        asm("move.l (%sp)+, %a5"); // A5
        asm("move.l (%sp)+, %a4"); // A4
        asm("move.l (%sp)+, %a3"); // A3
        asm("move.l (%sp)+, %a2"); // A2
        asm("move.l (%sp)+, %a1"); // A1
        asm("move.l (%sp)+, %a0"); // A0
    } else {
        rtx_dbug_outs("  Error: trying to switch to a process that is in an"
                      " unknown state\r\n");
    }
};
