#include "kernel.h"
#include "process.h"
#include "soft_interrupts.h"
#include "queues.h"
#include "../globals.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../rtx.h"

#define DEBUG_BLOCKED

extern message_queue delayed_messages;
extern int timer;

void* memory_head;
unsigned long int memory_alloc_field;
void* mem_end;

/**
 * Priority Queues
 */

process_queue ready_queue[NUM_PRIORITIES];
process_queue blocked_message_queue[NUM_PRIORITIES];
process_queue blocked_memory_queue[NUM_PRIORITIES];

process_queue* process_queues[] = {ready_queue,
                                   blocked_message_queue,
                                   blocked_memory_queue};

/**
 * @brief: System call used by a running process to release the processor.
 */

int k_release_processor() {
    process_control_block* process;
    
#ifdef DEBUG
    printf_0("k_release_processor()\r\n");
#endif

    if (!running_process->is_i_process) {
        if (running_process->state == STATE_BLOCKED_MESSAGE) {
            k_priority_enqueue_process(running_process, QUEUE_BLOCKED_MESSAGE);
        } else if (running_process->state == STATE_BLOCKED_MEMORY){
            k_priority_enqueue_process(running_process, QUEUE_BLOCKED_MEMORY);
        } else {
            k_priority_enqueue_process(running_process, QUEUE_READY);
        }
    }
 
    process = k_get_next_process(QUEUE_READY);

    return k_context_switch(process);
}

int k_preempt_processor(process_control_block* process) {

#ifdef DEBUG
    printf_1("k_preempt_processor(%x)\r\n", process);
#endif

    k_priority_insert_process(running_process, QUEUE_READY);
    return k_context_switch(process);
}

process_control_block* k_get_next_process(int queue) {
    int i;
    process_control_block* process;
    
    i = 0;
    process = NULL;
    while(i < NUM_PRIORITIES) {
        process = k_priority_dequeue_process(i, queue);
        if (process == NULL) {
            i++;
        } else {
            break;
        }
    }

    return process;
} 

process_control_block* k_peek_next_process(int queue) {
    int i;
    process_control_block* process;
    
    i = 0;
    process = NULL;
    while(i < NUM_PRIORITIES) {
        process = k_priority_dequeue_process(i, queue);
        k_priority_insert_process(process, queue);
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
    printf_0("k_get_process_priority()\r\n");
#endif 
    
    //
    // Check if the PID is valid
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
    printf_0("k_set_process_priority()\r\n");
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

    process = k_priority_queue_remove(pid);

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
    if (k_peek_next_process(QUEUE_READY)->priority < running_process->priority) {
        k_release_processor();
    }

    return RTX_SUCCESS;
}

//
// Memory
//

void* k_request_memory_block() {
    int block_index;
    void* block;
    
    //
    // Check if we have any free memory left. If not, return NULL
    //

    while (memory_head == NULL) {

        //
        // There is no memory available, switch out of this process
        // and set our state to blocked
        //

        #ifdef DEBUG_BLOCKED
        printf_1("No free memory: process %i is now blocked\r\n",
            running_process->pid);
        #endif

        running_process->state = STATE_BLOCKED_MEMORY;
        k_release_processor();
    }

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

int k_release_memory_block(void* memory_block) {
    int block_index;
    process_control_block* unblocked_process;

    //
    // Check the memory allocation field to see if this block has already
    // been deallocated.
    //

#ifdef DEBUG_MEM
    printf_1("Release_memory: MEMORY BLOCK = %x\r\n", memory_block);
#endif

    block_index = k_get_block_index(memory_block);
    
    if (memory_alloc_field & (0x01 << block_index)) {
      *(int*)memory_block = (int)memory_head;
      memory_head = memory_block;

      //
      // Update the allocated memory field
      //

      memory_alloc_field &= ((0x01 << block_index) ^ 0xFFFFFFFF);

      //
      // Success. Unblock the first process that is blocked on memory and move
      // it to the ready queue for its priority level
      //

      unblocked_process = k_get_next_process(QUEUE_BLOCKED_MEMORY);
      if (unblocked_process) {
          unblocked_process->state = STATE_READY;
          k_priority_enqueue_process(unblocked_process, QUEUE_READY);

 #ifdef DEBUG_BLOCKED
          printf_1("Process %i is now unblocked (memory)\r\n",
                   unblocked_process->pid);
#endif


          if (unblocked_process->priority < running_process->priority) {
              k_release_processor();
          }
      }

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
 
    return ((int)addr - (int)mem_end) / MEM_BLK_SIZE;
}

int k_send_message(int process_id, message_envelope* message) {
    process_control_block* receiving_process;
	
    if (process_id < 0 || process_id > NUM_PROCESSES - 1) {
        return RTX_ERROR;
    }

    receiving_process = &processes[process_id];
    
    //
    // Set the sender and receiver fields of the message
    //

    message->sender_pid = running_process->pid;
    message->receiver_pid = process_id;
    queue_enqueue_m(&receiving_process->messages, message);

    //
    // Update the state of the receiving process to be unblocked and move
    // it to the ready queue for its priority level.
    //

    if (receiving_process->state == STATE_BLOCKED_MESSAGE) {
        receiving_process->state = STATE_READY;      
        receiving_process = k_priority_queue_remove(process_id);
        k_priority_enqueue_process(receiving_process, QUEUE_READY);

#ifdef DEBUG_BLOCKED
        printf_1("Process %i is now unblocked (message)\r\n",
                 receiving_process->pid);
#endif

        if (receiving_process->priority < running_process->priority) {
            k_release_processor();
        }
    }

    //
    // The queue has no size limit, always return success
    //

    return RTX_SUCCESS;
}

int k_forward_message(message_envelope* message) {
    process_control_block* receiving_process;

    receiving_process = &processes[message->receiver_pid];
    queue_enqueue_m(&receiving_process->messages, message);

    //
    // Update the state of the receiving process to be unblocked and move
    // it to the ready queue for its priority level.
    //

    if (receiving_process->state == STATE_BLOCKED_MESSAGE) {
        receiving_process->state = STATE_READY;      
        receiving_process = k_priority_queue_remove(message->receiver_pid);
        k_priority_enqueue_process(receiving_process, QUEUE_READY);
    }

    //
    // The queue has no size limit, always return success
    //

    return RTX_SUCCESS;
}

void* k_receive_message(int* sender_id) {
    message_envelope* message;

    message = queue_dequeue_m(&running_process->messages);
    while (message == NULL) {

        //
        // The queue is empty, switch out of this process and set our state
        // to blocked
        //

        #ifdef DEBUG_BLOCKED
        printf_1("Message queue is empty: process %i is now blocked\r\n",
            running_process->pid);
        #endif

        running_process->state = STATE_BLOCKED_MESSAGE;
        k_release_processor();
        message = queue_dequeue_m(&running_process->messages);
    }


    if(sender_id) {
        *sender_id = message->sender_pid;
    }

    return message;
}

int k_delayed_send(int process_id, message_envelope* message, int delay) {
    if (process_id < 0 || process_id > NUM_PROCESSES-1 || delay < 0) {
        return RTX_ERROR;
    }

    message->sender_pid = running_process->pid;
    message->receiver_pid = process_id;
    message->delay = delay;
    message->delay_start = timer;
    queue_insert_m(&delayed_messages, message);
    
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
    printf_1("k_context_switch(pid = %i)\r\n", process->pid);
#endif

    if (!process) {
        printf_0("  Invalid process handle\r\n");
        return RTX_ERROR;
    }

    // 
    // Get current running process. If there is a process currently running,
    // perform a context switch.
    // 
    previous_process = running_process;
    if (previous_process) {

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
        
        if (previous_process->state == STATE_RUNNING) {
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
        printf_0("  Error: trying to switch to a process that is in an"
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

void k_priority_enqueue_process(process_control_block* process,
                                enum queue_type queue) {
    if (process != NULL) {
        queue_enqueue_p(&process_queues[queue][process->priority], process);
        process->queue = queue;
    }

    return; 
}

/**
 * @brief: Insert a process onto the appropriate priority queue
 * @param: process the process to enqueue
 * @param: queue the queue type to use
 */

void k_priority_insert_process(process_control_block* process,
                               enum queue_type queue) {
    if (process != NULL) {
        queue_insert_p(&process_queues[queue][process->priority], process);
        process->queue = queue;
    }

    return; 
}

/**
 * @brief: Dequeue the head of the priority param.
 * @params: priority the priority queue to dequeue from
 * @param: queue the queue type to use
 */

process_control_block* k_priority_dequeue_process(int priority, enum queue_type queue) {
    process_control_block* process = NULL;
    
    //
    // End early if the priority queue is empty or the priority level is
    // out of range. Priority is checked first to ensure that the head checking
    // is short-circuited.
    //

    if (priority >= 4 || priority < 0 || queue == QUEUE_NONE) {
        return NULL;
    }
    
    process = queue_dequeue_p(&process_queues[queue][priority]);
    if (process) {
        process->queue = QUEUE_NONE;
    }

    return process;
}

/**
 * @brief: Remove an arbitrary process from the queue it is in. Used for
 *         setting the process priority.
 * @param: pid the process pid to remove from the queues.
 * @param: queue the queue type to use
 */

process_control_block* k_priority_queue_remove(int pid) {
    process_control_block* process;

    //
    //  PID is out of range so break early.
    //
    
    if (pid < 0 || pid >= NUM_PROCESSES) {
        return NULL;     
    }

    process = &processes[pid];
    queue_remove_p(&process_queues[process->queue][process->priority], process);

    return process;
}

void k_init_priority_queues() {
    int i;
    for (i = 0; i < NUM_PRIORITIES; i++) {
        process_queues[QUEUE_READY][i].head = NULL;
        process_queues[QUEUE_READY][i].tail = NULL;
        process_queues[QUEUE_BLOCKED_MESSAGE][i].head = NULL;
        process_queues[QUEUE_BLOCKED_MESSAGE][i].tail = NULL;
        process_queues[QUEUE_BLOCKED_MEMORY][i].head = NULL;
        process_queues[QUEUE_BLOCKED_MEMORY][i].tail = NULL;
    }
}

#ifdef _DEBUG_HOTKEYS
/*
 * process_queue ready_queue[NUM_PRIORITIES];
 * process_queue blocked_message_queue[NUM_PRIORITIES];
 * process_queue blocked_memory_queue[NUM_PRIORITIES];
 */

int k_debug_prt_rdy_q() {
    return queue_debug_print(ready_queue);
}

int k_debug_prt_blk_mem_q() {
    return queue_debug_print(blocked_memory_queue);
}

int k_debug_prt_blk_rec_q() {
    return queue_debug_print(blocked_message_queue);
}

#endif /* _DEBUG_HOTKEYS */
