#include "rtx_inc.h"
#include "dbug.h"
#include "memory.h"
#include "init.h"
#include "kernel.h"
#include "rtx.h"
#include "loader/rtx_test.h"

extern void* __end;
extern void __REGISTER_TEST_PROCS_ENTRY__();

int __main(void) {
    return 0;
}

int main(void) {
    VOID* stack_start;    
    
    //
    // Disable interrupts
    //

    asm("move.w #0x2700, %sr");

    //
    // TODO: The stack should start after the area allocated
    // by the memory manager, not at the start of free memory.
    //

    stack_start = &__end;

    rtx_dbug_outs("Initializing processes...");
    init_processes(stack_start);
    rtx_dbug_outs("done.\r\n");

    rtx_dbug_outs("Initializing interrupts...");
    init_interrupts();
    rtx_dbug_outs("done.\r\n");

    //
    // Register the test processes. Required by test suite.
    //

    //__REGISTER_TEST_PROCS_ENTRY__();

    //
    // Change to null process (PID 0)
    //

    rtx_dbug_outs("Switching to null process...\r\n");
    k_switch_process(1);

    //
    // Enable interrupts
    // 

    asm("move.w #0x2000, %sr");

    return 0;
}


/**
 * @brief: Registration function used by test suite
 */  

void  __attribute__ ((section ("__REGISTER_RTX__"))) register_rtx() 
{
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
