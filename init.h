#ifndef _INIT_H_
#define _INIT_H_

#include "rtx_inc.h"

/**
 * Wrapper for all the initilization calls
 */

void init(void* stack_start);

/**
 * Initilization calls
 */

void init_processes(VOID* stack_start);
void init_interrupts();
void init_priority_queues();
void init_test_procs();
#endif 
