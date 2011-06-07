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
process_control_block* k_get_next_process();

/**
 * Internal queue calls 
 */

void k_init_priority_queues();
void k_priority_enqueue_process(process_control_block* process, enum queue_type queue);
process_control_block* k_priority_dequeue_process(int priority, enum queue_type queue);
process_control_block* k_priority_queue_remove(int pid, enum queue_type queue);

#endif
