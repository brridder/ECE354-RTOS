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

void process_test() {
    while (1) {
        rtx_dbug_outs("Test process run.\r\n");

        printf_0("   Test printf with no format\r\n");
        printf_0("   Test printf with no format, literal %%\r\n");
        printf_1("   Test printf with int format: %i\r\n", 0xFFFFFFFF);
        printf_1("   Test printf with hex format: %x\r\n", 0xDEADBEEF);
        printf_1("   Test printf with hex format: %x\r\n", 0xEFFFFFFF);
        printf_1("   Test printf with hex format: %x\r\n", 0xFFFFFFFA);

        set_process_priority(running_process->pid,3);
        if (get_process_priority(running_process->pid) == 3) {
            rtx_dbug_outs("   SUCCESS: Test Process priority has been changed.\r\n");
        } else {
            rtx_dbug_outs("   FAIL: Test Process priority has not been changed.\r\n");
        }

        set_process_priority(0,1);
        if (get_process_priority(0) == 4) {
            rtx_dbug_outs("   SUCCESS: Setting Null Process priority fail.\r\n");
        } else {
            rtx_dbug_outs("   FAIL: Null Process priority has been changed.\r\n");
        }

        if (set_process_priority(100, 1) == RTX_ERROR) {
            rtx_dbug_outs("   SUCCESS: Setting invalid PID returns error.\r\n");
        } else {
            rtx_dbug_outs("   FAIL: Setting invalid PID succeeds.\r\n");   
        }
        if (set_process_priority(1,4) == RTX_ERROR) {
            rtx_dbug_outs("   SUCCESS: Setting invalid priority returns error.\r\n");
        } else {
            rtx_dbug_outs("   FAIL: Setting invalid priority succeeds.\r\n");   
        }
        
        release_processor();        
    }
}

void process_test_get_priority() {
    while (1) {
        rtx_dbug_outs("Test process: Get_priority() run.\r\n");
        if (get_process_priority(10) == RTX_ERROR) {
            rtx_dbug_outs("   SUCCESS: Getting process priority with invalid PID returns error.\r\n");
        } else {
            rtx_dbug_outs("   FAIL: Getting process priority with invalid PID succeeds.\r\n");
        }
        release_processor();
    }
}
