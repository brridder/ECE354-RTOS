#include "kernel.h"

#include "dbug.h"
#include "process.h"
#include "globals.h"

int k_release_processor() {
    rtx_dbug_outs("k_release_processor()\r\n");
    return 0;
}

void k_change_process(process_control_block* process) {
    process_control_block* previous_process;

    // TODO: Null check on `process`

    // 
    // Get current running process
    // 

    previous_process = running_process;
    if (previous_process) {
        // TODO: Proper context switch
    }    

    //
    // Switch to the process
    //

    running_process = process;
    if (running_process->state == STATE_STOPPED) {

    } else if (running_process->state == STATE_PAUSED) {

    } else if (running_process->state == STATE_RUNNING) {

    } else {

        // 
        // Unknown state.
        // TODO: Handle this
        //
    }

    running_process->state = STATE_RUNNING;

    //
    // Restore stack pointer and registers from the stack
    //

    asm("move.l %0, %%a7" : : "m" (running_process->stack));

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
    // Return to user process execution
    //

    asm("rte");
};
