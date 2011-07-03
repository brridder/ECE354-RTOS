/**
 * @file: init.c
 * @brief: initilization functions for RTX
 * @author: Ben Ridder
 * @author: Casey Banner
 * @date: 05/24/2011
 */

#include "init.h"
#include "process.h"
#include "kernel.h"
#include "../globals.h"
#include "../lib/dbug.h"
#include "../processes/system_processes.h"
//#include "./soft_interrupts.h"
#include "../tests/rtx_test.h"
#include "../rtx.h"
#include "../lib/string.h"

extern void* memory_head;
extern unsigned long int memory_alloc_field;
extern void* mem_end;

// 
// Test processes info. Registration function provided by test script
// The __REGISTER_TEST_PROCS_ENTRY__ symbol is in the linker scripts
//

extern void __REGISTER_TEST_PROCS_ENTRY__();

/**
 * @brief: Handles all the initilization of the OS
 * @param: stack_start the start of free memory
 */
void init(void* memory_start) {

#ifdef DEBUG
    rtx_dbug_outs("Initilizating memory...");
#endif

    init_memory(memory_start);

#ifdef DEBUG
    rtx_dbug_outs("done\r\nInitilizating processes...");
#endif

    init_processes(memory_head);

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
    int i;
    //
    // Setup null process
    // Entry point is defined in system_processes.c 
    //
   
    init_test_procs();

    processes[0].pid = 0;
    processes[0].priority = 4;
    processes[0].stack_size = 4096; 
    processes[0].entry = &process_null;
    processes[0].is_i_process = FALSE;
    processes[0].next = NULL;
    processes[0].previous = NULL;

    //
    // Iterate through all processes and setup their stack and state
    //
    
    // 
    for (i = 0; i < NUM_PROCESSES; i++) {
        //
        // Setup the process' stack pointer. The stack grows downward,
        // so the stack pointer for each process must be set to the end of the
        // memory allocated for each process' stack.
        //        
        
        processes[i].stack = stack_start + processes[i].stack_size;

        //
        // Setup the process' stack with an exception frame which points
        // to the entry point of the process.
        //

        stack_iter = (int*)processes[i].stack;

        //
        // Exception frame used to start this process
        // See section 11.1.2 of Coldfire Family Programmer's Reference Manual
        //

        *(--stack_iter) = (int)processes[i].entry; // PC 
        *(--stack_iter) = 0x40000000; // SR

        //
        // Save the stack pointer
        // 
        
        processes[i].stack = (void*)stack_iter;

        // 
        // The process is currently stopped
        // 
        
        processes[i].state = STATE_STOPPED;

        //
        // Setup message queue
        // 
        
        processes[i].messages.head = NULL;
        processes[i].messages.tail = NULL;

        //
        // Update the location of the next stack and move to the next process
        //

        processes[i].queue = QUEUE_NONE;
        stack_start = stack_start + processes[i].stack_size;
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
    // Setup the timer to use auto-vectored interrupt level 6, priority 3, at 1ms
    //

    TIMER0_ICR = 0x9B;
    TIMER0_TRR = 25;
    TIMER0_TMR = 0xC71B;

    asm("move.l #timer_isr, %d0");
    asm("move.l %d0, 0x10000078");

    //
    // TODO: Setup UART
    //

    asm("move.l (%a7)+, %d0");
    asm("move.l (%a7)+, %a0");    
}

/**
 * @brief: Initialize the priority queues
 */

void init_priority_queues() {
    int i;
    k_init_priority_queues();
    for (i = 0; i < NUM_TEST_PROCS; i++) {
        k_priority_enqueue_process(&processes[i+1], QUEUE_READY);
    }
}

/**
 * @brief: Initialize test processes
 */

void init_test_procs() {
    int i;
    int pid;
    __REGISTER_TEST_PROCS_ENTRY__();
     
    for (i = 0; i < NUM_TEST_PROCS; i++) {
        pid = g_test_proc[i].pid;
        processes[pid].pid = pid;
        processes[pid].priority = 0;
        processes[pid].priority = g_test_proc[i].priority;
        processes[pid].stack_size = g_test_proc[i].sz_stack; 
        processes[pid].entry = g_test_proc[i].entry;
        processes[pid].is_i_process = FALSE;
        processes[pid].next = NULL;
        processes[pid].previous = NULL;
    }
}

void init_memory(void* memory_start) {
    int i;
    int *current_block;
    
    //
    // The head pointer starts at the start of free memory plus
    // the space required for all our memory blocks
    // Since this is pointer arithmetic, adding 1 adds 4 bytes.
    //
    
    mem_end = memory_start;
    memory_head = (void*)((int)memory_start + ((NUM_MEM_BLKS-1)*MEM_BLK_SIZE));

#ifdef DEBUG_MEM
    printf_1("MEMORY HEAD: %x\n\r", memory_head);
#endif

    //
    // Iterate through the memory pool and setup the free list.
    // The first 4 bytes of each memory block contain the address
    // of the next free memory block.
    //
   
    current_block = (int*)memory_head;
    for (i = 0; i < NUM_MEM_BLKS; i++) {
        *current_block = (int)current_block - MEM_BLK_SIZE;
        if (i == NUM_MEM_BLKS - 1) {
            *current_block = NULL;
            break;
        }
        current_block = (void*)(*current_block);
    }

    //
    // Setup the memory allocation field. Each bit in this field
    // represents one block in the pool. A value of 0 means 
    // the block has not been allocated, 1 means the block has been allocated.
    //
    
    memory_alloc_field = 0x00000000;
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
     
    g_test_fixture.send_message = send_message;
    g_test_fixture.receive_message = receive_message;
    g_test_fixture.request_memory_block = request_memory_block;
    g_test_fixture.release_memory_block = release_memory_block;
    //g_test_fixture.delayed_send = delayed_send;

    rtx_dbug_outs((CHAR *)"rtx: leaving register_rtx()\r\n");
}
