#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "rtx_inc.h"
#include "rtx.h"

typedef struct _message_queue {
    message_envelope* head;
    message_envelope* tail;
} message_queue;

enum process_state {
    STATE_RUNNING,
    STATE_READY,
    STATE_STOPPED,
    STATE_BLOCKED_MESSAGE,
    STATE_BLOCKED_MEMORY
};

enum queue_type {
    QUEUE_READY = 0,
    QUEUE_BLOCKED_MESSAGE = 1,
    QUEUE_BLOCKED_MEMORY = 2,
    QUEUE_NONE = 3
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

    message_queue messages;

    struct _process_control_block* next;
    struct _process_control_block* previous;
} process_control_block;

typedef struct _process_queue {
    process_control_block* head;
    process_control_block* tail;
} process_queue;

#endif
