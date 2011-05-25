#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "shared/rtx_inc.h"

enum process_state {
    STATE_RUNNING = 0,
    STATE_STOPPED
};

typedef struct _process_control_block {
  UINT8 pid;
  UINT8 priority; 
  
  UINT32 data_registers[8];
  UINT32 addr_registers[8];
  UINT32 pc_register;
  UINT32 sr_register;

  UINT32 start_addr;
  UINT32 end_addr;
  
  enum process_state state;

  struct _process_control_block* next;
} process_control_block;

#endif
