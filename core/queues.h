#ifndef _QUEUES_H_
#define _QUEUES_H_

#include "../rtx.h"
#include "process.h"

void queue_enqueue_p(process_queue* queue, process_control_block* process);
process_control_block* queue_dequeue_p(process_queue* queue);
void queue_remove_p(process_queue* queue, process_control_block* process);

void queue_enqueue_m(message_queue* queue, message_envelope* message);
message_envelope* queue_dequeue_m(message_queue* queue);

#endif
