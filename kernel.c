#include "kernel.h"

#include "globals.h"
#include "dbug.h"
#include "process.h"
#include "soft_interrupts.h"
#include "string.h"

/**
 * Priority Queues
 */

process_control_block* p_q_ready_h[NUM_PRIORITIES];
process_control_block* p_q_ready_t[NUM_PRIORITIES];

process_control_block* p_q_done_h[NUM_PRIORITIES];
process_control_block* p_q_done_t[NUM_PRIORITIES];

process_control_block** queues_h[] = {p_q_ready_h, p_q_done_h};
process_control_block** queues_t[] = {p_q_ready_t, p_q_done_t};

/**
 * @brief: System call used by a running process to release the processor.
 */

int k_release_processor() {
    int i;
    process_control_block* process;
    
#ifdef DEBUG
    rtx_dbug_outs("k_release_processor()\r\n");
#endif

    k_priority_enqueue_process(running_process, QUEUE_DONE);
 
    process = k_get_next_process();
   
    //
    // If we still don't have a null process, copy done queues to ready queues
    //
    
    if (process == NULL) {
        for (i = 0; i < NUM_PRIORITIES; i++) {
            queues_h[QUEUE_READY][i] = queues_h[QUEUE_DONE][i];
            queues_t[QUEUE_READY][i] = queues_t[QUEUE_DONE][i];
            queues_h[QUEUE_DONE][i] = NULL;
            queues_t[QUEUE_DONE][i] = NULL;
        }
        for (i = 0; i < NUM_PROCESSES; i++) {
            processes[i].queue = QUEUE_READY;
        }
        process = k_get_next_process();
    }

    return k_context_switch(process);
}

process_control_block* k_get_next_process() {
    int i = 0;
    process_control_block* process = NULL;

    while(i < NUM_PRIORITIES) {
        process = k_priority_dequeue_process(i, QUEUE_READY);
        if (process == NULL) {
            i++;
        } else {
            break;
        }
    }
    return process;
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

    if (running_process) {
      if (running_process->pid == pid) {
        running_process->priority = priority;
        goto k_set_process_priority_done;
      } 
    }
   
    // 
    // Process isn't being run, so remove it from its current priority queue,
    // change the priority and enqueue it back onto the appropriate queue.
    //

    process = k_priority_queue_remove(pid, processes[pid].queue);

#ifdef DEBUG
    printf_1("  process is: %x\r\n", process);
#endif

    if (process == NULL) {
      processes[pid].priority = priority;
    } else {
      process->priority = priority;
      k_priority_enqueue_process(process, process->queue);
    }

#ifdef DEBUG
    printf_1("  priority is %i, enqueing...\r\n", process->priority);
#endif


k_set_process_priority_done:
    return RTX_SUCCESS;
}

//
// MEMORY
//

void* k_request_memory_block() {
    int block_index;
    void* block;
    
    //
    // Check if we have any free memory left. If not, return NULL
    //

    if (memory_head != NULL) {
        //
        // Allocate the block on the top of the free list, moving
        // the head of the free list to the next available block
        //

        block = memory_head;
        memory_head = (void*)*(UINT32*)block;

        //
        // Set the bit in the memory field corresponding to this block
        // 

        block_index = k_get_block_index(block);
        memory_alloc_field |= (0x01 << block_index);

        return block;
    }
    
    return NULL;
}

int k_release_memory_block(void* memory_block) {
    int block_index;

    //
    // Check the memory allocation field to see if this block has already
    // been deallocated.
    //
    
    block_index = k_get_block_index(memory_block);
    if (memory_alloc_field & (0x01 << block_index)) {
      *(int*)memory_block = (int)memory_head;
      memory_head = memory_block;

      //
      // Update the allocated memory field
      //

      memory_alloc_field &= ((0x01 << block_index) ^ 0xFFFFFFFF);

      //
      // Success
      //

      return RTX_SUCCESS;
    }

    //
    // This memory block is not currently allocated, failure
    //
    
    return RTX_ERROR;
}

int k_get_block_index(void* addr) {
    //
    // The block index is its offset into the free memory region (in bytes)
    // divided by the block size.
    //
 
    return ((int)addr - (int)&mem_end) / MEM_BLK_SIZE;
}

int k_send_message(int process_id, void* message_envelope) {    
    return 0;
}

void* k_receive_message(int* sender_id) {
    return NULL;
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
        rtx_dbug_outs("  Invalid process handle\r\n");
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
 * @brief: Enqueue a process onto the appropriate priority queue
 * @param: process the process to enqueue
 * @param: queue the queue type to use
 */

void k_priority_enqueue_process(process_control_block* process, enum queue_type queue) {
#ifdef DEBUG
    rtx_dbug_outs("k_priority_enqueue_process()\r\n");    
    printf_0("  before enqueu\r\n");
    printf_1("  queu: %i\r\n", queue);

    if (process != NULL) {
      printf_1("  prio: %i\r\n", process->priority);
      printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
      printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
    }
#endif 

    //
    // The process is null, so break early.
    //
    
    if (process == NULL) {
        goto k_priority_enqueue_process_done;
    }
    
    if (queues_h[queue][process->priority] == NULL) {

        //
        // The queue is empty. Assign the head and tail pointers of the queue to
        // the process. 
        //
        
        queues_h[queue][process->priority] = process;
        queues_t[queue][process->priority] = process;
        process->previous = NULL;
        process->next = NULL;
    } else {

        // 
        // The queue is not empty. Attach the passed-in process to the current 
        // tail and update the tail pointer.
        //

        process->previous = queues_t[queue][process->priority];
        process->next = NULL;
        queues_t[queue][process->priority]->next = process;
        queues_t[queue][process->priority] = process;
    }

    process->queue = queue;

k_priority_enqueue_process_done:
#ifdef DEBUG
    printf_1("Enqueued process: %i\r\n", process->pid);
    printf_0("  after enqueu\r\n");
    if (process != NULL) {
      printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
      printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
    }
#endif

    return; 
}

/**
 * @brief: Dequeue the head of the priority param.
 * @params: priority the priority queue to dequeue from
 * @param: queue the queue type to use
 */

process_control_block* k_priority_dequeue_process(int priority, enum queue_type queue) {
    process_control_block* process = NULL;
#ifdef DEBUG
    rtx_dbug_outs("k_priority_dequeue_process()\r\n");
    printf_0("  before deenqueu\r\n");
    printf_1("  queu: %i\r\n", queue);
    printf_1("  prio: %i\r\n", priority);
    printf_1("  queues_h[queue][priority]: %x\r\n", queues_h[queue][priority]);
    printf_1("  queues_t[queue][priority]: %x\r\n", queues_h[queue][priority]);
#endif
    
    //
    // End early if the priority queue is empty or the priority level is
    // out of range. Priority is checked first to ensure that the head checking
    // is short-circuited.
    //

    if (priority >= 4 || priority < 0
        || queues_h[queue][priority] == NULL
        || queue == QUEUE_NONE) {
        return NULL;
    }
    
    if (queues_h[queue][priority]->next == NULL) {
        // 
        // Only one item on the queue
        //
        
        queues_t[queue][priority] = NULL;
        process = queues_h[queue][priority];
        queues_h[queue][priority] = NULL;
    } else {
        // 
        // Pop the head
        //
        
        process = queues_h[queue][priority];
        queues_h[queue][priority] = process->next;
        queues_h[queue][priority]->previous = NULL;
        process->next = NULL;
        process->previous = NULL;
    }   
    
    process->queue = QUEUE_NONE;

#ifdef DEBUG
    printf_1("Dequeued process: %i\r\n", process->pid);
    printf_0("  after deenqueu\r\n");
    printf_1("  queu: %i\r\n", queue);
    if (process != NULL) {
      printf_1("  prio: %i\r\n", process->priority);
      printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
      printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
    }
#endif

    return process;
}

/**
 * @brief: Remove an arbitrary process from the queue it is in. Used for
 *         setting the process priority.
 * @param: pid the process pid to remove from the queues.
 * @param: queue the queue type to use
 */

process_control_block* k_priority_queue_remove(int pid, enum queue_type queue) {
    process_control_block* process;

#ifdef DEBUG
    rtx_dbug_outs("k_priority_queue_remove()\r\n");
    printf_0("  before remove\r\n");
#endif

    //
    //  PID is out of range so break early.
    //
    
    if (pid < 0 || pid >= NUM_PROCESSES || queue == QUEUE_NONE) {
        return NULL;     
    }

    process = &processes[pid];

#ifdef DEBUG
    printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
    printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
    printf_1("  process is: %x\r\n", process);
    printf_1("  queue is: %i\r\n", process->queue);
    printf_1("  process->next is: %x\r\n", process->next);
    printf_1("  process->previous is: %x\r\n", process->previous);
    printf_1("  process->priority is: %i\r\n", process->priority);
#endif

    if (process->next == NULL && process->previous == NULL) { 
        // 
        // Process is the only item in the queue.  
        //
        
#ifdef DEBUG
      rtx_dbug_outs("  removing only queue item\r\n");
      printf_1("  priority: %i\r\n", process->priority);
      printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
      printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
#endif

        queues_h[queue][process->priority] = NULL;
        queues_t[queue][process->priority] = NULL;
    } else if (process->next == NULL) { 
        // 
        // Process is the tail
        //

#ifdef DEBUG
      rtx_dbug_outs("  removing queue tail\r\n");
#endif

        process->previous->next = NULL;
        queues_t[queue][process->priority] = process->previous;
    } else if (process->previous == NULL) { 
        // 
        // Process is the head
        //
      
#ifdef DEBUG
      rtx_dbug_outs("  removing queue head\r\n");
#endif

        process->next->previous = NULL;
        queues_h[queue][process->priority] = process->next;
    } else {
        // 
        // Process is in the middle somewhere.
        // 

#ifdef DEBUG
      rtx_dbug_outs("  removing item from middle of queue\r\n");
#endif

        process->next->previous = process->previous;
        process->previous->next = process->next;
    }

    process->next = NULL;
    process->previous = NULL;

#ifdef DEBUG
    printf_0("  after remove\r\n");
    printf_1("  queues_h[queue][process->priority]: %x\r\n", queues_h[queue][process->priority]);
    printf_1("  queues_t[queue][process->priority]: %x\r\n", queues_t[queue][process->priority]);
#endif

    return process;
}

void k_init_priority_queues() {
    int i;
    for (i = 0; i < NUM_PRIORITIES; i++) {
        queues_h[QUEUE_READY][i] = NULL;
        queues_t[QUEUE_READY][i] = NULL;
        queues_h[QUEUE_DONE][i] = NULL;
        queues_t[QUEUE_DONE][i] = NULL;
    }
}
