#include "i_processes.h"
#include "../globals.h"
#include "../uart/uart.h"
#include "../rtx.h"
#include "../core/queues.h"
#include "../lib/string.h"

char char_out;
int char_handled;

message_queue delayed_messages;
int timer;

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
            
            if (char_in == '\0') {
                continue;
            }

            in_string[i++] = char_in;

            if (char_in == CR) {
                need_new_line = 1;
                char_handled = 1;

                in_string[i++] = '\n';
                in_string[i++] = '\0';
               
                if (in_string[0] == '%') {
                    //
                    // Input was a command to a process so send it forward.
                    //
                    message = (message_envelope*)request_memory_block();
                    message->type = MESSAGE_KEY_INPUT;
                    str_cpy(message->data, in_string);
                    send_message(KCD_PID, message);
                    message = NULL;
                } else { // Fucking stupid hack to get shit to work properly
                    // 
                    // Echo back the CR and LF
                    //
                    message = (message_envelope*)request_memory_block();
                    message->type = MESSAGE_KEY_INPUT;
                    message->data[0] = '\n';
                    message->data[1] = '\0';
                    send_message(CRT_DISPLAY_PID, message);
                }

                i = 0;
            }
            // Echo it back.
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
