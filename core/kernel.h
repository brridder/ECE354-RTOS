#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"
#include "../globals.h"
#include "../rtx.h"
/**
 * Queue
 */

process_control_block* running_process;
process_control_block processes[NUM_PROCESSES];

/**
 * System call primitives
 */

int k_release_processor();
int k_get_process_priority(int pid);
int k_set_process_priority(int pid, int priority);

void* k_request_memory_block();
int k_release_memory_block(void* memory_block);

int k_send_message(int process_id, message_envelope* message);
int k_forward_message(message_envelope* message);
void* k_receive_message(int* sender_id);
int k_delayed_send(int process_id, message_envelope* message, int delay);

/**
 * Internal kernel calls
 */

int k_context_switch(process_control_block* process_control);
process_control_block* k_get_next_process();
int k_preempt_processor(process_control_block* process);

int k_get_block_index(void* addr);

/**
 * Internal queue calls 
 */

void k_init_priority_queues();
void k_priority_enqueue_process(process_control_block* process,
                                enum queue_type queue);
void k_priority_insert_process(process_control_block* process,
                                enum queue_type queue);
process_control_block* k_priority_dequeue_process(int priority,
                                                  enum queue_type queue);
process_control_block* k_priority_queue_remove(int pid);

#endif
