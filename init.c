/**
 * @file: init.c
 * @brief: initilization functions for RTX
 * @author: Ben Ridder
 * @author: Casey Banner
 * @date: 05/24/2011
 */

#include "init.h"
#include "dbug.h"
#include "kernel.h"
#include "system_processes.h"
#include "process.h"
#include "soft_interrupts.h"

#define PROCESS_NUM_REGISTERS 15

// 
// Test processes info. Registration function provided by test script
// The __REGISTER_TEST_PROCS_ENTRY__ symbol is in the linker scripts
//

extern void __REGISTER_TEST_PROCS_ENTRY__();

/**
 * @brief: Copies the process tables entries into the PCBs
 * @param: stack_start the starting memory location for stacks
 */

void init_processes(VOID* stack_start) {
    int* stack_iter;
    process_control_block* current_process;

    //
    // Setup null process
    // Entry point is defined in system_processes.c 
    //
    
    processes[2].pid = 2;
    processes[2].priority = 3;
    processes[2].stack_size = 4096; 
    processes[2].entry = &process_test_get_priority;
    processes[2].is_i_process = FALSE;
    processes[2].next = NULL;

    processes[1].pid = 1;
    processes[1].priority = 3;
    processes[1].stack_size = 4096; 
    processes[1].entry = &process_test;
    processes[1].is_i_process = FALSE;
    processes[1].next = &processes[2];

    processes[0].pid = 0;
    processes[0].priority = 4;
    processes[0].stack_size = 4096; 
    processes[0].entry = &process_null;
    processes[0].is_i_process = FALSE;
    processes[0].next = &processes[1];

    // TODO: Add all processes to process list

    //
    // Iterate through all processes and setup their stack and state
    //

    current_process = &processes[0];
    while (current_process) {
        //
        // Setup the process' stack pointer. The stack grows downward,
        // so the stack pointer for each process must be set to the end of the
        // memory allocated for each process' stack.
        //        
        
        current_process->stack = stack_start + current_process->stack_size;

        //
        // Setup the process' stack with an exception frame which points
        // to the entry point of the process.
        //

        stack_iter = (int*)current_process->stack;

        //
        // Exception frame used to start this process
        // See section 11.1.2 of Coldfire Family Programmer's Reference Manual
        //

        *(--stack_iter) = (int)current_process->entry; // PC 
        *(--stack_iter) = 0x40000000; // SR

        //
        // Save the stack pointer
        // 
        
        current_process->stack = (void*)stack_iter;

        // 
        // The process is currently stopped
        // 
        
        current_process->state = STATE_STOPPED;

        //
        // Update the location of the next stack and move to the next process
        // 

        stack_start = stack_start + current_process->stack_size;
        current_process = current_process->next;
    }

    //
    // No process is running
    //

    running_process = NULL;

    init_priority_queues();
}

/**
 * @brief: Initialize the VBR and install interrupts.
 */

void init_interrupts() {
    asm("move.l %a0, -(%a7)");
    asm("move.l %d0, -(%a7)");
    
    //
    // Initialize the VBR
    //
    
    asm("move.l #0x10000000, %a0");
    asm("movec.l %a0, %vbr");
	
    //
    // Install the system_call function into the first vector in the VBR
    //
    
    asm("move.l #system_call, %d0");
    asm("move.l %d0, 0x10000080");

    //
    // TODO: Setup UART and Timer interrupts
    //

    asm("move.l (%a7)+, %d0");
    asm("move.l (%a7)+, %a0");    
}

/**
 * @brief: Initialize the priority queues
 */

void init_priority_queues() {
    int i;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        priority_queue_heads[i] = NULL;
        priority_queue_tails[i] = NULL;
    }
       
    for (i = 1; i < 3; i++) {
        k_priority_enqueue_process(&processes[i]);
    }
}
