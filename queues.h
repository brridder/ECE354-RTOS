#ifndef _QUEUES_H_
#define _QUEUES_H_

#include "rtx.h"
#include "process.h"

void queue_enqueue_m(message_queue* queue,
                           message_envelope* message);
message_envelope* queue_dequeue_m(message_queue* queue);

#endif
