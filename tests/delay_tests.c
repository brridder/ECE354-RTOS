/*--------------------------------------------------------------------------
 *                      RTX Test Suite 
 *--------------------------------------------------------------------------
 */
/**
 * @file:   rtx_test_dummy.c   
 * @author: Thomas Reidemeister
 * @author: Irene Huang
 * @date:   2010.02.11
 * @brief:  rtx test suite 
 */

#include "rtx_test.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../rtx.h"

/* third party dummy test process 1 */ 
void test1()
{
    void* message_1;
    void* message_2;
    void* message_3;
    void* message_4;

    printf_0("rtx_test: test1\r\n");      

    message_1 = g_test_fixture.request_memory_block();
    delayed_send(2, message_1, 1);

    message_2 = g_test_fixture.request_memory_block();
    delayed_send(2, message_2, 100);

    message_3 = g_test_fixture.request_memory_block();
    delayed_send(2, message_3, 1000);

    message_4 = g_test_fixture.request_memory_block();
    delayed_send(2, message_4, 2000);
    printf_0("Process 1 done\r\n");

    while (1) {
        g_test_fixture.release_processor();
    }
}

void test2()
{
    int sender_id = -1;
    void* message;

    printf_0("rtx_test: test2\r\n");

    while (1) 
    {
        message = g_test_fixture.receive_message(&sender_id);
        g_test_fixture.release_memory_block(message);
        if (sender_id != -1) {
            printf_1("Process 2 received message from PID %i\r\n", sender_id);
            sender_id = -1;
        }
        
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 3 */ 
void test3()
{
    printf_0("rtx_test: test3\r\n");

    while (1) 
    {   
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    printf_0("rtx_test: test4\r\n");

    while (1) 
    {
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */ 
void test5()
{
    printf_0("rtx_test: test5\r\n");

    while (1) 
    {
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 6 */ 
void test6()
{
    printf_0("rtx_test: test6\r\n");

    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;

    printf_0("rtx_test: register_test_proc()\r\n");

    for (i = 0; i < NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
        g_test_proc[i].priority = 3;
        g_test_proc[i].sz_stack = 2048;
    }
    g_test_proc[0].entry = test1;
    g_test_proc[1].entry = test2;
    g_test_proc[2].entry = test3;
    g_test_proc[3].entry = test4;
    g_test_proc[4].entry = test5;
    g_test_proc[5].entry = test6;
}

/**
 * Main entry point for this program.
 * never get invoked
 */
int main(void)
{
    printf_0("rtx_test: started\r\n");
    return 0;
}
