#include "kernel.h"

#include "dbug.h"
#include "process.h"
#include "soft_interrupts.h"

/**
 * @brief: System call used by a running process to release the processor.
 */

int k_release_processor() {
    int current_priority;
    process_control_block* process;
    
#ifdef DEBUG
    rtx_dbug_outs("k_release_processor()\r\n");
#endif  
    //
    // TODO: Make a decision on what process to run
    //
    
    current_priority = running_process->priority;
    process = k_priority_dequeue_process(current_priority);

    while (process == NULL) {
        current_priority = (current_priority + 1) % 4;
        process = k_priority_dequeue_process(current_priority);
    }
    return k_switch_process(process->pid);
}

/**
 * @brief:
 * @param:
 */

int k_switch_process(int pid) {
    if (pid != 1 && pid != 2) {
        return RTX_ERROR;
    }
    if (running_process != NULL) {
        k_priority_enqueue_process(running_process);
    }   
    k_context_switch(&processes[pid]);
    return RTX_SUCCESS;
}

/**
 * @brief: Returns the priority of a process
 * @param: pid the pid of the process
 */
int k_get_process_priority(int pid) {

#ifdef DEBUG
    rtx_dbug_outs("k_get_process_priority()\r\n");
#endif 
    //
    // Invalid pid was passed in
    //

    if (pid >= NUM_PROCESSES) {
        return RTX_ERROR;
    }

    return processes[pid].priority;
}

/**
 * @brief: 
 * @params:
 */

int k_set_process_priority(int pid, int priority) {
    process_control_block* process;

#ifdef DEBUG
    rtx_dbug_outs("k_set_process_priority()\r\n");
#endif

    //
    // Invalid pid or invalid priority was passed in
    //

    if (pid >= NUM_PROCESSES || pid <= 0 || priority >= 4 || priority < 0) {
        return RTX_ERROR;
    }

    // If running
    if (running_process->pid == pid) {
        running_process->priority = priority;
        goto k_set_process_priority_done;
    } 
    
    process = k_priority_queue_remove(pid);
    process->priority = priority;
    k_priority_enqueue_process(process);
k_set_process_priority_done:
    return RTX_SUCCESS;
}

/**
 * @brief: Performs a context switch. After the context switch, the 
 *         process begins executing in user mode.
 * @param: process the process to switch to.
 */
int k_context_switch(process_control_block* process) {
    process_control_block* previous_process;

#ifdef DEBUG
    rtx_dbug_outs("k_context_switch()\r\n");
#endif

    if (!process) {
#ifdef DEBUG
        rtx_dbug_outs("  Invalid process handle\r\n");
#endif
        return RTX_ERROR;
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
        running_process->state = STATE_RUNNING;

        //
        // If the process has not been started yet, we need to start it by
        // calling `rte`. The exception frame generated in init_processes is 
        // the only thing on its stack.
        //

        asm("rte");
    } else if (running_process->state == STATE_READY) {
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
#ifdef DEBUG
        rtx_dbug_outs("  Error: trying to switch to a process that is in an"
                      " unknown state\r\n");
#endif
        return RTX_ERROR;
    }
    return RTX_SUCCESS;
}

/**
 * @brief:
 * @params:
 */

void k_priority_enqueue_process(process_control_block* process) {
    if (!process) {
        return;
    }
    if (!priority_queue_heads[process->priority]) {
        priority_queue_heads[process->priority] = process;
        priority_queue_tails[process->priority] = process;
        process->previous = NULL;
        process->next = NULL;
        return;
    } else {
        process->previous = priority_queue_tails[process->priority];
        process->next = NULL;
        priority_queue_tails[process->priority]->next = process;
        priority_queue_tails[process->priority] = process;
    }
}

/**
 * @brief:
 * @params:
 */

process_control_block* k_priority_dequeue_process(int priority) {
    process_control_block* process;
    
    if (priority_queue_heads[priority] == NULL 
        || priority >= 4 || priority < 0) {
        return NULL;
    }

    if (priority_queue_heads[priority] == priority_queue_tails[priority]) {
        priority_queue_tails[priority] = NULL;
        process = priority_queue_heads[priority];
        priority_queue_heads[priority] = NULL;
    } else {
        process = priority_queue_heads[priority];

        priority_queue_heads[priority] = process->next;
        priority_queue_heads[priority]->previous = NULL;

        process->next = NULL;
        process->previous = NULL;
    }
    return process;
}

process_control_block* k_priority_queue_remove(int pid) {
    process_control_block* process;
    if (pid < 0 || pid >= NUM_PROCESSES) {
        return NULL;     
    }
    process = &processes[pid];
    if (process->next == NULL && process->previous == NULL) { // Only item
        priority_queue_heads[process->priority] = NULL;
        priority_queue_tails[process->priority] = NULL;
    } else if (process->next == NULL) { // TAIL
        process->previous->next = NULL;
        priority_queue_tails[process->priority] = process->previous;
    } else if (process->previous == NULL) { // HEAD
        process->next->previous = NULL;
        priority_queue_heads[process->priority] = process->next;
    } else {
        process->next->previous = process->previous;
        process->previous->next = process->next;
    }
    return process;
}

