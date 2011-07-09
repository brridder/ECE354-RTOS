#include "queues.h"
#include "../lib/string.h"

void queue_insert_p(process_queue* queue, process_control_block* process) {
    if (queue->head == NULL && queue->tail == NULL) {
        queue_enqueue_p(queue, process);
    } else {

        // 
        // The queue is not empty. Attach the process to the current 
        // head and update the head pointer.
        //

        process->previous = NULL;
        process->next = queue->head;
        queue->head->previous = process;
        queue->head = process;
    }
}

void queue_enqueue_p(process_queue* queue, process_control_block* process) {
    if (queue->head == NULL && queue->tail == NULL) {

        //
        // The queue is empty. Assign the head and tail pointers of the queue to
        // the process. 
        //

        queue->head = process;
        queue->tail = process;        
        process->previous = NULL;
        process->next = NULL;
    } else {

        // 
        // The queue is not empty. Attach the process to the current 
        // tail and update the tail pointer.
        //
        
        process->previous = queue->tail;
        process->next = NULL;
        queue->tail->next = process;
        queue->tail = process;
    }
}

process_control_block* queue_dequeue_p(process_queue* queue) {
    process_control_block* process;
    process = queue->head;
    if (process) {
        if (process->next == NULL) {

            // 
            // Only one item on the queue
            //

            queue->head = NULL;
            queue->tail = NULL;            
        } else {
            queue->head = process->next;
            queue->head->previous = NULL;
            process->next = NULL;
            process->previous = NULL;
        }
    } else {
        process = NULL;
    }

    return process;
}

void queue_remove_p(process_queue* queue, process_control_block* process) {
    if (process->next == NULL && process->previous == NULL) { 

        // 
        // Process is the only item in the queue.  
        //        

        queue->head = NULL;
        queue->tail = NULL;
    } else if (process->next == NULL) { 

        // 
        // Process is the tail
        //

        process->previous->next = NULL;
        queue->tail = process->previous;
    } else if (process->previous == NULL) { 
        // 
        // Process is the head
        //
      
        process->next->previous = NULL;
        queue->head = process->next;
    } else {
        // 
        // Process is in the middle somewhere.
        //

        process->next->previous = process->previous;
        process->previous->next = process->next;
    }

    process->next = NULL;
    process->previous = NULL;
}

void queue_enqueue_m(message_queue* queue, message_envelope* message) {
    if (queue->head == NULL) {
        
        // 
        // The receiver's queue is empty
        //

        queue->head = message;
        queue->tail = message;
        message->previous = NULL;
        message->next = NULL;
    } else {

        //
        // Message queue is not empty, enqueue message
        //
        
        message->previous = queue->tail;
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
            queue->head->previous = NULL;
            message->next = NULL;
            message->previous = NULL;
        }

        return message;
    } else {

        //
        // Receiver's queue is empty
        //

        return NULL;
    }
}

void queue_remove_m(message_queue* queue, message_envelope* message) {
    if (message->next == NULL && message->previous == NULL) { 

        // 
        // Message is the only item in the queue.  
        //        

        queue->head = NULL;
        queue->tail = NULL;
    } else if (message->next == NULL) { 

        // 
        // Message is the tail
        //

        message->previous->next = NULL;
        queue->tail = message->previous;
    } else if (message->previous == NULL) { 
        // 
        // Message is the head
        //
      
        message->next->previous = NULL;
        queue->head = message->next;
    } else {
        // 
        // Message is in the middle somewhere.
        //

        message->next->previous = message->previous;
        message->previous->next = message->next;
    }

    message->next = NULL;
    message->previous = NULL;
}


/**
 * @brief inserts a message into a queue, sorted ascending by
 *        message->delay+message->delay_start
 */
void queue_insert_m(message_queue* queue, message_envelope* message) {
    message_envelope* message_iter;

    if (queue->head == NULL && queue->tail == NULL) {
        queue_enqueue_m(queue, message);
    } else {        
        message_iter = queue->head;
        while (message_iter) {
            if ((message->delay + message->delay_start) <=
                (message_iter->delay + message_iter->delay_start)) {

                //
                // Insert message before message_iter
                //
            
                message->next = message_iter;
                message->previous = message_iter->previous;
                message_iter->previous = message;
                
                if (message->previous) {
                    message->previous->next = message;
                }

                if (message_iter == queue->head) {
                    queue->head = message;
                }

                goto queue_insert_m_done;
            }

            message_iter = message_iter->next;
        }

        //
        // Largest value, put on tail
        //

        queue_enqueue_m(queue, message);
    }

queue_insert_m_done:
    return;
}

