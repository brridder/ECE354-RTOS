/** 
 * @file: rtx.h
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */

#ifndef _RTX_H_
#define _RTX_H_

/*
 * Processor Management
 */

int release_processor();

/*
 * Process Priority
 */

int set_process_priority(int pid, int priority);
int get_process_priority(int pid);

/*
 * Memory Management
 */

void* request_memory_block();
int release_memory_block(void* memory_block);

/*
 * Interprocess Communication
 */

int send_message(int process_id, void* message_envelope);
void* receive_message(int* sender_id);

#endif
