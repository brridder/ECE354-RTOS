#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "shared/rtx_inc.h"

enum process_state {
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_STOPPED
};

typedef struct _process_control_block {
    UINT8 pid;
    UINT8 priority;

    VOID* stack;
    UINT32 stack_size;

    void (*entry)();
    BOOLEAN is_i_process;

    enum process_state state;

    struct _process_control_block* next;
} process_control_block;

#endif
