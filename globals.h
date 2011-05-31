#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "process.h"
#include "rtx_config_table.h"

// TODO :: move to a constants.h file
#define NUM_PROCESSES 6 

process_control_block* running_process;
process_control_block processes[NUM_PROCESSES];

#endif
