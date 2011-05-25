#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "shared/rtx_inc.h"

typedef struct _process_control_block {
  uint8_t pid;
  uint8_t priority;
  
  uint32_t data_registers[8];
  uint32_t addr_registers[8];
  uint32_t pc_register;
  uint32_t sr_register;

  uint32_t start_addr;
  uint32_t end_addr;

  process_control_block* next;
} process_control_block;

#endif
