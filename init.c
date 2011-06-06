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
#include "loader/rtx_test.h"
#include "rtx.h"
#include "string.h"


#define PROCESS_NUM_REGISTERS 15

// 
// Test processes info. Registration function provided by test script
// The __REGISTER_TEST_PROCS_ENTRY__ symbol is in the linker scripts
//

extern void __REGISTER_TEST_PROCS_ENTRY__();
/**
 * @brief: Handles all the initilization of the OS
 * @param: stack_start the starting memory location for process stacks
 */

void init(void* stack_start) {
#ifdef DEBUG
    rtx_dbug_outs("Initilizating processes...");
#endif
    init_processes(stack_start);
#ifdef DEBUG
    rtx_dbug_outs(" done\r\nInitializing priority queues...");
#endif
    init_priority_queues();
#ifdef DEBUG
    rtx_dbug_outs(" done\r\nInitilizating interrupts...");
#endif
    init_interrupts();
#ifdef DEBUG
    rtx_dbug_outs(" done\r\n");
#endif
}

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
   /* 
    processes[NUM_TEST_PROCS + 2].pid = NUM_TEST_PROCS + 2;
    processes[NUM_TEST_PROCS + 2].priority = 3;
    processes[NUM_TEST_PROCS + 2].stack_size = 4096; 
    processes[NUM_TEST_PROCS + 2].entry = &process_test_get_priority;
    processes[NUM_TEST_PROCS + 2].is_i_process = FALSE;
    processes[NUM_TEST_PROCS + 2].next = NULL;

    processes[NUM_TEST_PROCS + 1].pid = NUM_TEST_PROCS + 1;
    processes[NUM_TEST_PROCS + 1].priority = 3;
    processes[NUM_TEST_PROCS + 1].stack_size = 4096; 
    processes[NUM_TEST_PROCS + 1].entry = &process_test;
    processes[NUM_TEST_PROCS + 1].is_i_process = FALSE;
    processes[NUM_TEST_PROCS + 1].next = &processes[2];
   */ 
    init_test_procs();

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
        printf_1("Loading up processes: %i\r\n", current_process->pid);
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
        printf_1("Done: %i\r\n", current_process->pid);
        current_process = current_process->next;
        if (current_process->pid > 6) { 
            // TODO :: FIX THIS PROPERLY
            current_process = NULL;
        } else {
            printf_1("Next: %i\r\n", current_process->pid);
        }
    }

    //
    // No process is running
    //
    printf_0("Done!\r\n");
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

/**
 * @brief: Initialize the priority queues
 */

void init_priority_queues() {
    int i;

    for (i = 0; i < NUM_PRIORITIES; i++) {
        priority_queue_heads[i] = NULL;
        priority_queue_tails[i] = NULL;
    }
       
    for (i = 0; i < NUM_TEST_PROCS-1; i++) {
        k_priority_enqueue_process(&processes[i+1]);
    }
}

/**
 * @brief: Initialize test processes
 */

void init_test_procs() {
    // TODO :: make this more robust
//#ifdef NUM_TEST_PROCS 
    int i;
    __REGISTER_TEST_PROCS_ENTRY__();
    
    for (i = 0; i < NUM_TEST_PROCS; i++) {
        processes[i+1].pid = g_test_proc[i].pid;
        processes[i+1].priority = g_test_proc[i].priority;
        processes[i+1].stack_size = g_test_proc[i].sz_stack; 
        processes[i+1].entry = g_test_proc[i].entry;
        processes[i+1].is_i_process = FALSE;
        if (i == NUM_TEST_PROCS - 1) {
            processes[i+1].next = NULL;
        } else {
            processes[i+1].next = &processes[i+2];
        }
    }
//#endif
}

/**
 * @brief: Registration function used by test suite
 */  

void  __attribute__ ((section ("__REGISTER_RTX__"))) register_rtx() {
    rtx_dbug_outs((CHAR *)"rtx: Entering register_rtx()\r\n");
    
    g_test_fixture.release_processor = release_processor;
    g_test_fixture.set_process_priority = set_process_priority;
    g_test_fixture.get_process_priority = get_process_priority;
    //
    // TODO: Implement required OS functions
    //
     
    //g_test_fixture.send_message = send_message;
    //g_test_fixture.receive_message = receive_message;
    //g_test_fixture.request_memory_block = request_memory_block;
    //g_test_fixture.release_memory_block = release_memory_block;
    //g_test_fixture.delayed_send = delayed_send;

    rtx_dbug_outs((CHAR *)"rtx: leaving register_rtx()\r\n");
}
