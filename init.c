/**
 * @file: init.c
 * @brief: initilization functions for RTX
 * @author: Ben Ridder
 * @author: Casey Banner
 * @date: 05/24/2011
 */

#include "init.h"
#include "globals.h"
#include "dummy/rtx_test.h"
#include "system_processes.h"
#include "process.h"
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

void init_processes(VOID* stack_start) {
    process_control_block* current_process;

    //
    // Setup null process
    // Entry point is defined in system_processes.c 
    //
    
    processes[0].pid = 0;
    processes[0].priority = 4; 
    processes[0].stack_size = 1024; 
    processes[0].entry = &process_null;
    processes[0].is_i_process = FALSE;
    processes[0].next = NULL;

    current_process = &processes[0];

    // TODO: Setup all processes

    while (current_process) {
        //
        // Setup the process' stack pointer. The stack grows downward,
        // so the stack pointer for each process must be set to the end of the
        // memory allocated for each processes stack.
        //        
        
        current_process->stack = stack_start + current_process->stack_size;

        //
        // Setup the process' stack with values of 0 for each register,
        // and the exception frame which points to the entry point 
        // of the process.
        // 
        //

        asm("move.l %a0, -(%a7)");
        asm("move.l %d0, -(%a7)");

        asm("move.l %0, %%a0" : : "m" (current_process->stack));

        //
        // Exception frame
        //

        asm("move.l %0, -(%%a0)" : : "m" (&(current_process->entry))); // PC
        asm("move.l #0x40000000, -(%a0)"); // F/V and SR

        //
        // Registers
        //
        
        asm("move.l #0, -(%a0)"); // A0        
        asm("move.l #0, -(%a0)"); // A1
        asm("move.l #0, -(%a0)"); // A2
        asm("move.l #0, -(%a0)"); // A3
        asm("move.l #0, -(%a0)"); // A4
        asm("move.l #0, -(%a0)"); // A5
        asm("move.l #0, -(%a0)"); // A6
        asm("move.l #0, -(%a0)"); // D0
        asm("move.l #0, -(%a0)"); // D1
        asm("move.l #0, -(%a0)"); // D2
        asm("move.l #0, -(%a0)"); // D3
        asm("move.l #0, -(%a0)"); // D4
        asm("move.l #0, -(%a0)"); // D5
        asm("move.l #0, -(%a0)"); // D6
        asm("move.l #0, -(%a0)"); // D7

        //
        // Save stack pointer
        //

        asm("move.l %a0, %d0");
        asm("move.l %%d0, %0" : "=m" (current_process->stack));

        asm("move.l (%a7)+, %d0");
        asm("move.l (%a7)+, %a0");
        
        // 
        // All processes are currently stopped
        // 
        
        processes[0].state = STATE_STOPPED;

        //
        // Update the location of the next stack and move to the next process
        // 

        current_process = current_process->next;
        stack_start = stack_start + current_process->stack_size;
    }

    //
    // No process is running
    //

    running_process = NULL;
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
