#include "init.h"
#include "globals.h"
#include "dummy/rtx_test.h"
#include "system_processes.h"
#include "process_control_block.h"

void init_processes(UINT32 stack_start) {
    int i,j;
    for(i = 0; i < NUM_PROCESSES; i++) {
        pcbs[i].pid = proc_table[i].pid;
        pcbs[i].priority = proc_table[i].priority;
        for(j = 0; j < 8; j++) {
            pcbs[i].data_registers[j] = 0;
            pcbs[i].addr_registers[j] = 0;
        }
        pcbs[i].addr_registers[7] = stack_start; // Stack pointer
        stack_start = stack_start + proc_table[i].stack_size; // Next starting location of the stack
        pcbs[i].pc_register = 0;
        pcbs[i].sr_register = 0;
        
        pcbs[i].start_addr = (int)(proc_table[i].proc_entry);
        pcbs[i].end_addr = 0; // TODO :: How do we find this?

        pcbs[i].state = STATE_STOPPED;
        pcbs[i].next = NULL; // For now.
    }

}

void init_rtx_process_tables() {
}

void init_null_process() {
    proc_table[0].pid = 0;
    proc_table[0].priority = 4;
    proc_table[0].stack_size = 4096; // TODO :: make smaller?
    proc_table[0].proc_entry = &null_process;
    proc_table[0].is_i_process = FALSE;
}
