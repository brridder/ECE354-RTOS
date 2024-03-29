/** 
 * @file: rtx.c
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */
 
#include "rtx.h"
#include "./lib/dbug.h"
#include "./core/kernel.h"
#include "./core/soft_interrupts.h"

int release_processor() {
    return do_system_call(CALL_RELEASE_PROCESSOR, (void*)0, 0);
}

int set_process_priority(int pid, int priority) {
    int args[2];
    args[0] = pid;
    args[1] = priority;
    return do_system_call(CALL_SET_PROCESS_PRIORITY, args, 2);
}

int get_process_priority(int pid) {
    int args[1];
    args[0] = pid;

    return do_system_call(CALL_GET_PROCESS_PRIORITY, args, 1);
}

void* request_memory_block() {
    return (void*)do_system_call(CALL_REQUEST_MEM_BLK, (void*)0, 0);
}

int release_memory_block(void* memory_block) {
    int args[1];
    args[0] = (int)memory_block;
    
    return do_system_call(CALL_RELEASE_MEM_BLK, args, 1);
}

int send_message(int process_id, void* message_envelope) {
    int args[2];
    args[0] = process_id;
    args[1] = (int)message_envelope;

    return do_system_call(CALL_SEND_MESSAGE, args, 2);
}

void* receive_message(int* sender_id) {
    int args[1];
    args[0] = (int)sender_id;

    return (void*)do_system_call(CALL_RECEIVE_MESSAGE, args, 1);
}

int delayed_send(int process_id, void* message_envelope, int delay) {
    int args[3];
    args[0] = process_id;
    args[1] = (int)message_envelope;
    args[2] = delay;

    return do_system_call(CALL_DELAYED_SEND, args, 3);
}

#ifdef _DEBUG_HOTKEYS
int debug_prt_rdy_q() {
    return do_system_call(CALL_DEBUG_PRT_RDY_Q, 0, 0);
}

int debug_prt_blk_mem_q() {
    return do_system_call(CALL_DEBUG_PRT_BLK_MEM, 0, 0);
}

int debug_prt_blk_rec_q() {
    return do_system_call(CALL_DEBUG_PRT_BLK_REC, 0, 0);
}

int debug_prt_mem_blks_free() {
    return do_system_call(CALL_DEBUG_PRT_MEM_BLKS_FREE, 0, 0);
}

int debug_prt_message_history() {
    return do_system_call(CALL_DEBUG_PRT_MESSAGE_HISTORY, 0, 0);
}
#endif

