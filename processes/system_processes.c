/**
 * @file: system_processes.c
 * @brief: defines the system processes used by RTX
 * @author: Ben Ridder
 * @date: 05/24/2011
 */

#include "system_processes.h"
#include "../core/process.h"
#include "../rtx.h"
#include "../core/kernel.h"
#include "../core/queues.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../globals.h"
#include "../uart/uart.h"

//#define KCD_DEBUG

char char_out;
int char_handled;

message_queue delayed_messages;
int timer;

/**
 * @brief: null_process that does nothing
 */

void process_null() {
    while (1) {
        rtx_dbug_outs("Null process run.\r\n");

        // TODO :: Remove when we get preemption working
        release_processor();
    }
}

void i_process_uart() {
    uart_interrupt_config inter_cfg;
    message_envelope *message;
    unsigned char uart_state;
    int need_new_line;
    char in_string[64];
    char char_in;
    int i;

    i = 0;
    message = NULL;
    inter_cfg.tx_rdy = true;
    need_new_line = 0; 

    while(1) {
        uart_state = SERIAL1_USR;

        // 
        // Read in waiting data
        //
        
        if (uart_state & 0x01) {
            char_in = SERIAL1_RD;
            in_string[i++] = char_in;

            if (char_in == CR) {
                need_new_line = 1;
                char_handled = 1;
                
                in_string[i++] = '\n';
                in_string[i++] = '\0';
               
                if (in_string[0] == '%') {
                    message = (message_envelope*)request_memory_block();
                    message->type = MESSAGE_KEY_INPUT;
                    str_cpy(message->data, in_string);
                    send_message(KCD_PID, message);
                    message = NULL;
                } else { // Fucking stupid hack to get shit to work properly
                    message = (message_envelope*)request_memory_block();
                    message->type = MESSAGE_KEY_INPUT;
                    message->data[0] = '\n';
                    message->data[1] = '\0';
                    send_message(CRT_DISPLAY_PID, message);
                }

                i = 0;
            }
            char_out = char_in;
            uart1_set_interrupts(&inter_cfg);
#ifdef UART_DEBUG
            printf_1("uart1 char in : %i\r\n", char_in);
#endif
        //
        // Print out data
        //
        } else if (uart_state & 0x04) {
            char_handled = 0;
            SERIAL1_WD = char_out;
            SERIAL1_IMR = 0x02;
        }

        release_processor();
    }
}

void i_process_timer() {    
    message_envelope* current_message;
    
    timer = 0;
    delayed_messages.head = NULL;
    delayed_messages.tail = NULL;

    while(1) {
        release_processor();

        //
        // Increment timer
        //
        
        timer++;
        
        #ifdef DEBUG
        if (timer % 1000 == 0) {
            printf_1("Timer: %ims\r\n", timer);
        }
        #endif

        //
        // Check for delayed messages that need to be sent
        //

        if (delayed_messages.head != NULL) {
            while ((timer - delayed_messages.head->delay_start) >=
                   delayed_messages.head->delay) {
                current_message = delayed_messages.head;

                #ifdef DEBUG
                printf_1("Message with delay: %i ready, forwarding\r\n", 
                         current_message->delay);
                #endif
                
                //
                // Remove from queue and deliver
                //

                queue_remove_m(&delayed_messages, current_message);
                k_forward_message(current_message);
            }
        }
    }
}

void process_crt_display() {
    int i;
    int sender_id;
    message_envelope *message;
    uart_interrupt_config int_config;
    
    int_config.tx_rdy = true;
    message = NULL;
    i = 0;
    char_handled = 0;

    while(1) {
        message = receive_message(&sender_id);
        if (message->type == MESSAGE_OUTPUT || 
            message->type == MESSAGE_KEY_INPUT) { 
            i = 0;
            while (message->data[i] != '\0') { 
                if (!char_handled) {
                    char_handled = 1;
                    char_out = message->data[i++];        
                    uart1_set_interrupts(&int_config);
                }
            }
        }
        release_memory_block(message);
        message = NULL;
        
       release_processor();
    }
}

// 
// Keyboard command decoder
//
void process_kcd() {
    int sender_id;
    int num_cmds;
    int i;
    int j;
    char str[64];
    message_envelope *message_receive; 
    message_envelope *message_send; 
    command cmds[32]; 
    
    num_cmds = 0;

    while(1) {
        message_receive = (message_envelope*)receive_message(&sender_id);
        if (message_receive->type == MESSAGE_KEY_INPUT) {
            for(i = 0; i < num_cmds; i++) {
                if (message_receive->data[1] == cmds[i].cmd_str[1]) {
                    message_send = (message_envelope*)request_memory_block();
                    str_cpy(message_send->data, message_receive->data);
                    send_message(cmds[i].reg_pid, message_send);
#ifdef KCD_DEBUG
                    printf_1("Found it for pid: %i\r\n", cmds[i].reg_pid);
#endif
                    break;
                }
            }
            send_message(CRT_DISPLAY_PID, message_receive);
        } else if (message_receive->type == MESSAGE_CMD_REG) {
            str_cpy(cmds[num_cmds].cmd_str, message_receive->data);
            cmds[num_cmds].reg_pid = sender_id;
            num_cmds++;
            release_memory_block(message_receive);
#ifdef KCD_DEBUG
            printf_1("Registered for %i\r\n",  cmds[num_cmds-1].reg_pid);
#endif
        } else {
            release_memory_block(message_receive);
        }
        message_receive = NULL;
        release_processor();
    }
}
