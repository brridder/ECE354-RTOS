#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "process.h"

int k_release_processor();
void k_change_process(process_control_block* process_control);

#endif
