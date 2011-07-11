#include "user_processes.h"
#include "../rtx.h"
#include "../lib/string.h"
#include "../core/queues.h"
#include "../core/process.h"

#define USER_DEBUG
void proc_a() {
    const unsigned char command[] = "%Z";
    message_envelope* message;
    int num;
    
    num = 0;
    //
    // Register the command: %Z
    //
    message = (message_envelope*)request_memory_block(); 
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, (char*)command);
    send_message(KCD_PID, message);

    while(1) {
        //
        // Loop until we get %Z  (AKA, idle until called upon)
        //
        message = (message_envelope*)receive_message(0);
        if (str_cmp(message->data, command)) {
            release_memory_block(message);
            break;
        } else {
            release_memory_block(message);
        }
    }
    
    while(1) {
        //
        // Send messages to process B forever.
        //
        message = (message_envelope*)request_memory_block();
        message->type = MESSAGE_COUNT_REPORT;
        memcpy(message->data, &num, sizeof(num));
        send_message(PROC_B, message);
        num++;
        release_processor();
    }
}

void proc_b() {
    message_envelope* message;
    while(1) {
        //
        // Forward messages to process C
        //
        message = (message_envelope*)receive_message(0);
        send_message(PROC_C, message);
    }
}

void proc_c() {
    const unsigned char output[] = "Process C\r";
    message_envelope* message_in;
    message_envelope* message_delay; 
    message_queue queue;
    queue.head = NULL;
    queue.tail = NULL;
    int error_code;
    int message_data;

    while(1) {
        //
        // If local message queue is null, get another message from the kernel
        //
        if (queue.head == NULL) {
            message_in = (message_envelope*)receive_message(0);
        } else {
            message_in = queue_dequeue_m(&queue); 
        }

        //
        // Check message type
        //
        if (message_in->type == MESSAGE_COUNT_REPORT) {

            //
            // Copy the message data into a temp buffer
            // 
            
            memcpy(&message_data, message_in->data, sizeof(message_data));

            //
            // If the counter is divisible by 20, print out 'PROCESS C' and
            // delay the next loop by 10 seconds while enqueueing all incoming
            // messages
            //
            
            if (message_data % 20 == 0 && message_data != 0) {
                message_in->type = MESSAGE_KEY_INPUT;
                str_cpy(message_in->data, (char*)output);
                send_message(CRT_DISPLAY_PID, message_in);

                message_delay = (message_envelope*)request_memory_block();
                message_delay->type = MESSAGE_WAKE_UP_10;
                delayed_send(PROC_C, message_delay, 10000);

                while(1) {
                    message_in = (message_envelope*)receive_message(0);
                    if (message_in->type == MESSAGE_WAKE_UP_10) {
                        break;
                    } else {
                        queue_enqueue_m(&queue, message_in); 
                    }
                }
            }
        }

        //
        // Release the recieved message
        //

        error_code = release_memory_block(message_in);

#ifdef USER_DEBUG
        if (error_code == RTX_ERROR) {
            printf_0("Performed a double deallocation in Process C\r\n");
        }
#endif
        release_processor();
    }
}

