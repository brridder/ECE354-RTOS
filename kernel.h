#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"

/**
 * Global kernel state variables.
 */ 

#define NUM_PROCESSES 6

process_control_block* running_process;
process_control_block processes[NUM_PROCESSES];

int k_release_processor();
void k_change_process(process_control_block* process_control);

#endif
