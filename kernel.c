#include "kernel.h"

#include "dbug.h"
#include "process.h"

/**
 * @brief: System call used by a running proccess to release the processor.
 */
int k_release_processor() {
    rtx_dbug_outs("k_release_processor()\r\n");
    
    //
    // TODO: Make a decision on what process to run
    //

    //k_change_process(running_process);

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
    int* stack_iter;
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
            
            stack_iter = (int*)previous_process->stack;
        
            //
            // Save register contents. It is expected that the exception
            // frame has already been saved onto the user process' stack.
            //

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
        
            previous_process->stack = (void*)stack_iter;
            previous_process->state = STATE_READY;
        }
    }    

    //
    // Switch to the process
    //

    running_process = process;
    running_process->state = STATE_RUNNING;

    //
    // Restore stack pointer and registers from the stack
    //

    asm("move.l %0, %%sp" : : "r" (running_process->stack));

    asm("move.l (%a7)+, %d7"); // D7
    asm("move.l (%a7)+, %d6"); // D6
    asm("move.l (%a7)+, %d5"); // D5
    asm("move.l (%a7)+, %d4"); // D4
    asm("move.l (%a7)+, %d3"); // D3
    asm("move.l (%a7)+, %d2"); // D2
    asm("move.l (%a7)+, %d1"); // D1
    asm("move.l (%a7)+, %d0"); // D0
    asm("move.l (%a7)+, %a6"); // A6
    asm("move.l (%a7)+, %a5"); // A5
    asm("move.l (%a7)+, %a4"); // A4
    asm("move.l (%a7)+, %a3"); // A3
    asm("move.l (%a7)+, %a2"); // A2
    asm("move.l (%a7)+, %a1"); // A1
    asm("move.l (%a7)+, %a0"); // A0

    //
    // Return to user process execution. We do not need to UNLK the frame 
    // pointer because we are using the user process' frame pointer at this
    // point. The `rte` will remove the exception frame from the stack.
    //

    asm("rte");
};
