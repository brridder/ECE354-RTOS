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
    while(1) {
        printf_0("UART FIRED");
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
