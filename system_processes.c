/**
 * @file: system_processes.c
 * @brief: defines the system processes used by RTX
 * @author: Ben Ridder
 * @date: 05/24/2011
 */
#include "system_processes.h"
#include "rtx.h"

/**
 * @brief: null_process that does nothing
 */

void null_process() {
    while (1) {
        // TODO :: Remove when we get preemption working
        release_processor(); 
    }
}
