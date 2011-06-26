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
#include "../dbug.h"
#include "../string.h"
#include "../rtx.h"

/* third party dummy test process 1 */ 
void test1()
{
    void * message;

    rtx_dbug_outs((CHAR *)"rtx_test: test1\r\n");      

    while (1) 
    {
       message = g_test_fixture.request_memory_block();
       if (message != NULL) {
           //message->data[0] = 50;
           //message->data[1] = 51;
           //message->data[2] = 52;
           //message->data[3] = 53;

          send_message(2, message);
        }

        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    int* sender_id;
    void* message_ptr;

    rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
    
    while (1) 
    {
        message_ptr = g_test_fixture.receive_message(sender_id);
        printf_1("Message received!, %i\r\n", sender_id);
        g_test_fixture.release_memory_block(message_ptr);
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 3 */ 
void test3()
{
    void* message;
    rtx_dbug_outs((CHAR *)"rtx_test: test3\r\n");
    while (1) 
    {   
        message = g_test_fixture.request_memory_block();
        g_test_fixture.send_message(5, message);
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    void* message;
    rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
    while (1) 
    {
        message = g_test_fixture.request_memory_block();
        g_test_fixture.send_message(5, message);
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */ 
void test5()
{
    int* sender_id = NULL;
    void* message = NULL;
    rtx_dbug_outs((CHAR *)"rtx_test: test5\r\n");
    while (1) 
    {
        message = g_test_fixture.receive_message(sender_id);
        if (sender_id == 3) {
            g_test_fixture.release_memory_block(message);
            message = g_test_fixture.receive_message(sender_id);
            g_test_fixture.release_memory_block(message);
            if (sender_id == 4) {
                printf_0("      SUCCESS: Received messages in order!\r\n");
            } else {
                printf_0("      FAIL: Received messages in order!\r\n");
            }
        } else {
            printf_1("      FAIL: Received wrong first message. Sender id %x\r\n", sender_id);
        }
        
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 6 */ 
void test6()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test6\r\n");
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;

    rtx_dbug_outs((CHAR *)"rtx_test: register_test_proc()\r\n");

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
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
    return 0;
}
