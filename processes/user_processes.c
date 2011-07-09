#include "user_processes.h"
#include "../rtx.h"
#include "../lib/string.h"

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
            //break;
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
    while(1) {
        release_processor();
    }
}

void proc_c() {
    while(1) {
        release_processor();
    }
}

