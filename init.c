/**
 * @file: init.c
 * @brief: initilization functions for RTX
 * @author: Ben Ridder
 * @date: 05/24/2011
 */

#include "init.h"
#include "globals.h"
#include "dummy/rtx_test.h"
#include "system_processes.h"
#include "process_control_block.h"
#include "soft_interrupts.h"

// 
// Test processes info. Registration function provided by test script
// The __REGISTER_TEST_PROCS_ENTRY__ symbol is in the linker scripts
//
extern void __REGISTER_TEST_PROCS_ENTRY__();

/**
 * @brief: Copies the process tables entries into the PCBs
 * @param: stack_start the starting memory location for stacks
 */

void init_processes(UINT32 stack_start) {
    int i,j;

    // 
    // Copy the process tables into the process control blocks.
    // *pcbs and *proc_table are defined in globals.h 
    //

    for(i = 0; i < NUM_PROCESSES; i++) {
        pcbs[i].pid = proc_table[i].pid;
        pcbs[i].priority = proc_table[i].priority;
        for(j = 0; j < 8; j++) {
            //
            // Set all the registers to 0
            //
            pcbs[i].data_registers[j] = 0;
            pcbs[i].addr_registers[j] = 0;
        }

        // 
        // Set the stack pointer to the stack_start memory location
        //

        pcbs[i].addr_registers[7] = stack_start; 

        //
        // Calculate the next starting location of the stack using the stack
        // size of the current process.
        //

        stack_start = stack_start + proc_table[i].stack_size;

        // 
        // Zero out the pc and sr starting values
        //
        
        pcbs[i].pc_register = 0;
        pcbs[i].sr_register = 0;
        
        //
        // Set the entry point of the function
        // TODO :: Change this to a proper function pointer
        // TODO :: Remove end_addr ?
        //
        
        pcbs[i].start_addr = (int)(proc_table[i].proc_entry);
        pcbs[i].end_addr = 0; 
        
        //
        // Set the state to stopped and the next process to none.
        // This will be handled by the scheduler
        //
        
        pcbs[i].state = STATE_STOPPED;
        pcbs[i].next = NULL;
    }
}

void init_interrupts() {
	asm("move.l %a0, -(%a7)");
	asm("move.l %d0, -(%a7)");
	
	//
	// Init the VBR
	//
	
    asm("move.l #0x10000000, %a0");
    asm("movec.l %a0, %vbr");
	
	//
	// Move the sys_call function into the first vector in the VBR
	//
	
	asm("move.l #sys_call, %d0");
	asm("move.l %d0, 0x10000080"); // TODO :: Make less magic?
	
	asm("move.l (%a7)+, %d0");
	asm("move.l (%a7)+, %a0");
	
	//
	// Other shit; like timers and uart0 and junk
	//
}

void init_rtx_process_tables() {
    int i;

    __REGISTER_TEST_PROCS_ENTRY__();
    //
    // Copy the test_process_table into the RTX proc_table
    // Assuming we are using the rtx_test_dummy.c file
    // This may change.
    //
    for(i = 0; i < NUM_TEST_PROCS; i++) {
        proc_table[i+1].pid = g_test_proc[i].pid;
        proc_table[i+1].priority = g_test_proc[i].priority;
        proc_table[i+1].stack_size = g_test_proc[i].sz_stack;
        proc_table[i+1].proc_entry = g_test_proc[i].entry;
    }
}

/**
 * @brief: Set the null process into the process table with pid of 0 and
 * priority 4
 */

void init_null_process() {
    proc_table[0].pid = 0;
    proc_table[0].priority = 4;
    proc_table[0].stack_size = 1024; // TODO :: make smaller? DOES IT REALLY NEED A STACK?

    //
    // Set the process_entry to the null_process function defined in
    // "system_processes.c"
    //
    
    proc_table[0].proc_entry = &null_process;
    proc_table[0].is_i_process = FALSE;
}
