#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "rtx_inc.h"

enum process_state {
    STATE_RUNNING,
    STATE_READY,
    STATE_STOPPED,
    STATE_BLOCKED_MESSAGE
};

enum queue_type {
    QUEUE_READY = 0,
    QUEUE_BLOCKED = 1,
    QUEUE_NONE = 2
};

typedef struct _process_control_block {
    int pid;
    int priority;

    void* stack;
    int stack_size;

    void (*entry)();
    BOOLEAN is_i_process;

    enum process_state state;

    enum queue_type queue;

    struct _process_control_block* next;
    struct _process_control_block* previous;
} process_control_block;

#endif
