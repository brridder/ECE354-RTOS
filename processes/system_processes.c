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
    //uart_interrupt_config *config;

    while(1) {
        uart_state = SERIAL1_USR;
        if (uart_state & 0x01) {
            char_in = SERIAL_RD;
        } else if (uart_state & 0x04) {
    
        }

        SERIAL1_IMR = 0x02;
        uart_in = SERIAL1_RD;
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
