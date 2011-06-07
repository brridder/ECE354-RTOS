#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"

/**
 * Global kernel state variables.
 */ 

#define NUM_PROCESSES 7
#define NUM_PRIORITIES 4


/**
 * Queue
 */

typedef enum _queue_type {
    QUEUE_READY = 0,
    QUEUE_DONE = 1
} queue_type;


process_control_block* running_process;
process_control_block processes[NUM_PROCESSES];

/**
 * System calls :: primatives
 */

int k_release_processor();
int k_get_process_priority(int pid);
int k_set_process_priority(int pid, int priority);

/**
 * Internal kernel calls
 */

int k_context_switch(process_control_block* process_control);
int k_switch_process(int pid);

/**
 * Internal queue calls 
 */

void k_init_priority_queues();
void k_priority_enqueue_process(process_control_block* process, queue_type queue);
process_control_block* k_priority_dequeue_process(int priority, queue_type queue);
process_control_block* k_priority_queue_remove(int pid, queue_type queue);

#endif
