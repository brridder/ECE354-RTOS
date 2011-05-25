#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "process_control_block.h"
#include "rtx_config_table.h"

// TODO :: move to a constants.h file
#define NUM_PROCESSES 6 
process_control_block pcbs[NUM_PROCESSES];
rtx_process_table proc_table[NUM_PROCESSES];
#endif
