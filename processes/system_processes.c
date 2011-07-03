/**
 * @file: system_processes.c
 * @brief: defines the system processes used by RTX
 * @author: Ben Ridder
 * @date: 05/24/2011
 */

#include "system_processes.h"
#include "../rtx.h"
#include "../core/kernel.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../globals.h"
#include "../uart/uart.h"

char char_out;
int char_handled;

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
    message_envelope *message;
    unsigned char uart_state;
    char in_string[64];
    char char_in;
    int i;

    i = 0;
    message = NULL;
    while(1) {
        uart_state = SERIAL1_USR;

        // 
        // Read in waiting data
        //
       
       if (uart_state & 0x01) {
            char_in = SERIAL1_RD;
            in_string[i++] = char_in;

            if (char_in == CR) {
                in_string[i++] = '\n';
                in_string[i++] = '\0';
                if (in_string[0] == '%') {
                    message = (message_envelope*)request_memory_block();
                    i = 0;
                    while (in_string[i] != '\0') {
                        message->data[i] = in_string[i];
                        i++;
                    }
                    send_message(KCD_PID, message);
                    message = NULL;
                } 

                i = 0; 
            }
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
    int timer = 0;
    
    printf_0("Timer process started\r\n");

    while(1) {
        release_processor();

        timer++;

        if (timer % 1000 == 0) {
            printf_1("Timer: %i\r\n", timer);
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
    printf_0("CRT DISPLAY_PROCESS Started\r\n");

    while(1) {
        message = receive_message(&sender_id);
        i = 0;
        while (message->data[i] != '\0') { 
            if (!char_handled) {
                char_handled = 1;
                char_out = message->data[i++];        
                uart1_set_interrupts(&int_config);
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
    message_envelope *message; 

    printf_0("KCD PROCESS STARTED\r\n");

    while(1) {
        message = (message_envelope*)receive_message(&sender_id);
        send_message(CRT_DISPLAY_PID, message);
        message = NULL;
        release_processor();
    }
}
