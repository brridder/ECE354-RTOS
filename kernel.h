#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"

/**
 * Global kernel state variables.
 */ 

#define NUM_PROCESSES 6
#define NUM_PRIORITIES 4

process_control_block* running_process;
process_control_block processes[NUM_PROCESSES];

process_control_block* priority_queue_heads[NUM_PRIORITIES];
process_control_block* priority_queue_tails[NUM_PRIORITIES];

int k_context_switch(process_control_block* process_control);
int k_switch_process(int pid);
/**
 * System calls
 */

int k_release_processor();
int k_get_process_priority(int pid);
int k_set_process_priority(int pid, int priority);

/**
 * Queue calls 
 */

void k_priority_enqueue_process(process_control_block* process);
process_control_block* k_priority_dequeue_process(int priority);
process_control_block* k_priority_queue_remove(int pid);
#endif
