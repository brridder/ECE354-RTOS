/** 
 * @file: rtx.h
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */

#ifndef _RTX_H_
#define _RTX_H_

#include "globals.h"
/*
 * Message Envelope
 */
enum message_type {
    MESSAGE_CMD_REG = 0,
    MESSAGE_KEY_INPUT,
    MESSAGE_OUTPUT,
    MESSAGE_OUTPUT_NO_NEWLINE,
    MESSAGE_COUNT_REPORT,
    MESSAGE_WAKE_UP_10
};

typedef struct _message_envelope {
    int sender_pid;
    int receiver_pid;
    enum message_type type;
    struct _message_envelope* next;
    struct _message_envelope* previous;
    int delay;
    int delay_start;
    unsigned char padding[36];
    unsigned char data[64];  
} message_envelope;



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
int delayed_send(int process_id, void* message_envelope, int delay);

#ifdef _DEBUG_HOTKEYS

int debug_prt_rdy_q();
int debug_prt_blk_mem_q();
int debug_prt_blk_rec_q();
int debug_prt_mem_blks_free();
int debug_prt_message_history();

#endif /*_DEBUG_HOTKEYS*/

#endif /* _RTX_H_ */
