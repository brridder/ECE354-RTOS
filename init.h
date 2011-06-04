#ifndef _INIT_H_
#define _INIT_H_

#include "rtx_inc.h"

void init_processes(VOID* stack_start);
void init_interrupts();
void init_priority_queues();

#endif 
