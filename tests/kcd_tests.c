/*--------------------------------------------------------------------------
 *                      RTX Test Suite 
 *--------------------------------------------------------------------------
 */
/**
 */

#include "rtx_test.h"
#include "../rtx.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../globals.h"

void test1() {
    message_envelope *message; 
    char* str = "%T";
    int sender_id;
    int i;

    message = (message_envelope*)g_test_fixture.request_memory_block();
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, str);

    send_message(KCD_PID, message); 
    
    rtx_dbug_outs((CHAR *)"rtx_test: test1\r\n");
    while (1) {
        message = (message_envelope*)g_test_fixture.receive_message(&sender_id);
        printf_0("Received %T");
        g_test_fixture.release_memory_block(message);    
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
    while (1) {
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 3 */ 
void test3()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test3\r\n");
    while (1) 
    {
        /* execute a rtx primitive to test */
        //printf_0("3\r\n");
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
    while (1) 
    {
        //printf_0("4\r\n");
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */ 
void test5()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test5\r\n");
    while (1) 
    {
        //printf_0("5\r\n");
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 6 */ 
void test6()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test6\r\n");
    while (1) 
    {
        //printf_0("6\r\n");
        /* execute a rtx primitive to test */
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
