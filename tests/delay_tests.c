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
#include "../globals.h"

void test1()
{
    void* message_1;
    void* message_2;
    void* message_3;
    void* message_4;
    void* message_5;
    void* message_6;

    printf_0(GID"_test: START\r\n"GID"_test: total 6 tests\r\n");

    message_1 = g_test_fixture.request_memory_block();
    delayed_send(2, message_1, 2);

    message_2 = g_test_fixture.request_memory_block();
    delayed_send(2, message_2, 100);

    message_3 = g_test_fixture.request_memory_block();
    delayed_send(2, message_3, 1000);

    message_4 = g_test_fixture.request_memory_block();
    delayed_send(2, message_4, 2000);

    message_5 = g_test_fixture.request_memory_block();
    delayed_send(2, message_5, 500);

    message_6 = g_test_fixture.request_memory_block();
    delayed_send(2, message_6, 1);

    while (1) {
        g_test_fixture.release_processor();
    }
}

void test2()
{
    const int delays[] = {1, 2, 100, 500, 1000, 2000};
    const int num_messages = 6;
    int message_num;
    int sender_id;
    void* message;
    
    message_num = 0;
    while (1) 
    {
        message = g_test_fixture.receive_message(&sender_id);
        printf_1("Process 2 expected message with delay %i...",
                 delays[message_num]);            
        if (((message_envelope*)message)->delay == delays[message_num]) {
            printf_0("success.\r\n");
        } else { 
            printf_0("fail.\r\n");
        }
        
        message_num++;
        
        if (message_num == num_messages) {
            printf_1("Process 2 received %i messages total...success.\r\n",
                     num_messages);
            printf_0(GID"_test: END\r\n");
        } else if (message_num > num_messages) {
            printf_1("Process 2 received over %i messages total...fail.\r\n",
                     num_messages);
        }

        g_test_fixture.release_memory_block(message);        
        g_test_fixture.release_processor();
    }
}

void test3()
{
    while (1) 
    {   
        g_test_fixture.release_processor();
    }
}

void test4()
{
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

void test5()
{
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

void test6()
{
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

//
// Register the third party test processes with RTX
//

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
