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

char in_string[1024];
char char_out;

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
    unsigned char uart_state;
    char char_in;
    int i;

    i = 0;

    while(1) {
        uart_state = SERIAL1_USR;
        
        // 
        // Read in waiting data
        //
       
       if (uart_state & 0x01) {
            char_in = SERIAL1_RD;
            in_string[i++] = char_in;
            if (char_in == CR) {
                in_string[i++] = '\0';
                i = 0; 
                if (in_string[0] == '%') {
                    printf_0("Switch to CRT DISPLAY PROC\r\n");
                    // CALL CRT DISPLAY PROCESS
                }
            }
#ifdef UART_DEBUG
            printf_1("uart1 char in : %i\r\n", char_in);
#endif
        //
        // Print out data
        //
        } else if (uart_state & 0x04) {
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
