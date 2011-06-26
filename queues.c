#include "queues.h"

void queue_enqueue_m(message_queue* queue, message_envelope* message) {
    if (queue->head == NULL) {
        
        // 
        // The receiver's queue is empty
        //

        queue->head = message;
        queue->tail = message;
        message->prev = NULL;
        message->next = NULL;
    } else {

        //
        // Message queue is not empty, enqueue message
        //
        
        message->prev = queue->tail;
        message->next = NULL;
        queue->tail->next = message;
        queue->tail = message;
    }
}

message_envelope* queue_dequeue_m(message_queue* queue) {
    message_envelope* message;

    message = queue->head;
    if (message) {
        if (message->next == NULL) {

            //
            // Only thing on queue
            //

            queue->head = NULL;
            queue->tail = NULL;
        } else {
            queue->head = message->next;
            queue->head->prev = NULL;
            message->next = NULL;
            message->prev = NULL;
        }

        return message;
    } else {

        //
        // Receiver's queue is empty
        //

        return NULL;
    }
}
