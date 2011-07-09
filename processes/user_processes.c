#include "user_processes.h"
#include "../rtx.h"
#include "../lib/string.h"
#include "../core/queues.h"
#include "../core/process.h"

void proc_a() {
    const unsigned char command[] = "%Z";
    message_envelope* message;
    int num;
    
    num = 0;

    message = (message_envelope*)request_memory_block(); 
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, (char*)command);
    send_message(KCD_PID, message);

    while(1) {
        message = (message_envelope*)receive_message(0);
        if (str_cmp(message->data, command)) {
            printf_0("GOT %Z\r\n");
            release_memory_block(message);
            break;
        } else {
            release_memory_block(message);
        }
    }
    
    while(1) {
        message = (message_envelope*)request_memory_block();
        message->type = MESSAGE_COUNT_REPORT;
        message->data[0] = (char)num;
        send_message(PROC_B, message);
        num++;
        release_processor();
    }
}

void proc_b() {
    message_envelope* message;
    while(1) {
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

    while(1) {
        if (queue.head == NULL) {
            message_in = (message_envelope*)receive_message(0);
        } else {
            message_in = queue_dequeue_m(&queue); 
        }

        if (message_in->type == MESSAGE_COUNT_REPORT) {
            if ((int)(message_in->data[0]) % 20 == 0) {
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
        error_code = release_memory_block(message_in);
#ifdef USER_DEBUG
        if (error_code == RTX_ERROR) {
            printf_0("Performed a double deallocation in Process C\r\n");
        }
#endif
        release_processor();
    }
}

