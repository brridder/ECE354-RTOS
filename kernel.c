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
    // TODO: Make the decision process better
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
 * @brief: Returns the priority of a process
 * @param: pid the pid of the process
 */

int k_get_process_priority(int pid) {
#ifdef DEBUG
    rtx_dbug_outs("k_get_process_priority()\r\n");
#endif 
    
    //
    // An invalid pid was passed in
    //

    if (pid >= NUM_PROCESSES || pid < 0) {
        return RTX_ERROR;
    }

    return processes[pid].priority;
}

/**
 * @brief: Set the priority of a process
 * @param: PID the pid of the process
 * @param: priority the priority to set the process at
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

    //
    // If the running process is the one to change, update it but with no need 
    // to change the priority queues.
    //

    if (running_process->pid == pid) {
        running_process->priority = priority;
        goto k_set_process_priority_done;
    } 
   
    // 
    // Process isn't being run, so remove it from its current priority queue,
    // change the priority and enqueue it back onto the appropriate queue.
    //
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
//#ifdef DEBUG
        rtx_dbug_outs("  Invalid process handle\r\n");
//#endif
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
 * @brief: Kernel function to facilitate process switching
 * @param: pid the pid of the process
 */

int k_switch_process(int pid) {
#ifdef DEBUG
    rtx_dbug_outs("k_switch_process()\r\n");
#endif
//    if (pid != 1 && pid != 2) {
 //       return RTX_ERROR;
  //  }
    if (running_process != NULL) {
        k_priority_enqueue_process(running_process);
    } else {
        k_priority_dequeue_process(3); // REMOVE THIS
    }
    k_context_switch(&processes[pid]);
    return RTX_SUCCESS;
}

/**
 * @brief: Enqueue a process onto the appropriate priority queue
 * @param: process the process to enqueue
 */

void k_priority_enqueue_process(process_control_block* process) {
#ifdef DEBUG
    rtx_dbug_outs("k_priority_enqueue_process()\r\n");
#endif 
    
    //
    // The process is null, so break early.
    //
    
    if (!process) {
        goto k_priority_enqueue_process_done;
    }
    
    if (!p_q_ready_h[process->priority]) {

        //
        // The queue is empty. Assign the head and tail pointers of the queue to
        // the process. 
        //
        
        p_q_ready_h[process->priority] = process;
        p_q_ready_t[process->priority] = process;
        process->previous = NULL;
        process->next = NULL;
    } else {

        // 
        // The queue is not empty. Attach the passed-in process to the current 
        // tail and update the tail pointer.
        //

        process->previous = p_q_ready_t[process->priority];
        process->next = NULL;
        p_q_ready_t[process->priority]->next = process;
        p_q_ready_t[process->priority] = process;
    }

k_priority_enqueue_process_done:
    return; 
}

/**
 * @brief: Dequeue the head of the priority param.
 * @params: priority the priority queue to dequeue from
 */

process_control_block* k_priority_dequeue_process(int priority) {
    process_control_block* process;
#ifdef DEBUG
    rtx_dbug_outs("k_priority_dequeue_process()\r\n");
#endif 
    
    //
    // End early if the priority queue is empty or the priority level is
    // out of range. Priority is checked first to ensure that the head checking
    // is short-circuited.
    //

    if (priority >= 4 || priority < 0 
        || p_q_ready_h[priority] == NULL) {
        return NULL;
    }
    
    if (p_q_ready_h[priority]->next == NULL) {
        // 
        // Only one item on the queue
        //
        
        p_q_ready_t[priority] = NULL;
        process = p_q_ready_h[priority];
        p_q_ready_h[priority] = NULL;
    } else {
        // 
        // Pop the head
        //
        
        process = p_q_ready_h[priority];
        p_q_ready_h[priority] = process->next;
        p_q_ready_h[priority]->previous = NULL;
        process->next = NULL;
        process->previous = NULL;
    }   
    return process;
}

/**
 * @brief: Remove an arbitrary process from the queue it is in. Used for
 *         setting the process priority.
 * @param: pid the process pid to remove from the queues.
 */

process_control_block* k_priority_queue_remove(int pid) {
    process_control_block* process;

#ifdef DEBUG
    rtx_dbug_outs("k_priority_queue_remove()\r\n");
#endif 

    //
    //  PID is out of range so break early.
    //
    
    if (pid < 0 || pid >= NUM_PROCESSES) {
        return NULL;     
    }
    
    process = &processes[pid];

    if (process->next == NULL && process->previous == NULL) { 
        // 
        // Process is the only item in the queue.  
        //
        
        p_q_ready_h[process->priority] = NULL;
        p_q_ready_t[process->priority] = NULL;
    } else if (process->next == NULL) { 
        // 
        // Process is the tail
        //
        
        process->previous->next = NULL;
        p_q_ready_t[process->priority] = process->previous;
    } else if (process->previous == NULL) { 
        // 
        // Process is the tail
        //
        
        process->next->previous = NULL;
        p_q_ready_h[process->priority] = process->next;
    } else {
        // 
        // Process is in the middle somewhere.
        // 
        
        process->next->previous = process->previous;
        process->previous->next = process->next;
    }

    return process;
}

