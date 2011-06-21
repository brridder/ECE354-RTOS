#ifndef _SOFT_INTERRUPTS_H_
#define _SOFT_INTERRUPTS_H_

enum system_calls {
    CALL_RELEASE_PROCESSOR = 0,
    CALL_GET_PROCESS_PRIORITY,
    CALL_SET_PROCESS_PRIORITY,
    CALL_REQUEST_MEM_BLK,
    CALL_RELEASE_MEM_BLK,
    CALL_SEND_MESSAGE,
    CALL_RECEIVE_MESSAGE
};

int do_system_call(int call_id, int* args, int num_args);
void system_call();

#endif
