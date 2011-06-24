#ifndef _SOFT_INTERRUPTS_H_
#define _SOFT_INTERRUPTS_H_

enum system_calls {
    CALL_RELEASE_PROCESSOR = 0,
    CALL_GET_PROCESS_PRIORITY = 1,
    CALL_SET_PROCESS_PRIORITY = 2,
    CALL_REQUEST_MEM_BLK = 3,
    CALL_RELEASE_MEM_BLK = 4,
    CALL_SEND_MESSAGE = 5,
    CALL_RECEIVE_MESSAGE = 6
};

int do_system_call(int call_id, int* args, int num_args);
void system_call();

#endif
