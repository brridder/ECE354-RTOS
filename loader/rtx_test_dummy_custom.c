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
#include "dbug.h"
#include "../string.h"

/* third party dummy test process 1 */ 
void test1()
{
    int i;
    rtx_dbug_outs((CHAR *)"rtx_test: test1\r\n");
    
    while (1) 
    {
        /* execute a rtx primitive to test */
        printf_0("1\r\n");
        i = g_test_fixture.get_process_priority(2);
        printf_1("Getting priority of pid 2: %i\r\n", i );
        printf_0("Setting priority of pid 2 to 3\r\n");
        g_test_fixture.set_process_priority(2,3);
        printf_1("Getting priority of pid 2: %i\r\n", g_test_fixture.get_process_priority(2));
        printf_1("Setting priority of pid 2 to %i\r\n", i);
        g_test_fixture.set_process_priority(2,i);
        printf_1("Getting priority of pid 2: %i\r\n", g_test_fixture.get_process_priority(2));
        printf_0("Next test should be pid 2\r\n");
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    int i;
    rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
    while (1) 
    {
        printf_0("2\r\n");
        i = g_test_fixture.get_process_priority(3);
        printf_1("Getting priority of pid 3: %i\r\n", i );
        printf_0("Setting priority of pid 3 to 3\r\n");
        g_test_fixture.set_process_priority(3,3);
        printf_1("Getting priority of pid 3: %i\r\n", g_test_fixture.get_process_priority(3));
        printf_1("Setting priority of pid 3 to %i\r\n", i);
        g_test_fixture.set_process_priority(3,i);
        printf_1("Getting priority of pid 3: %i\r\n", g_test_fixture.get_process_priority(3));
        printf_0("Next test should be pid 4\r\n");
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 3 */ 
void test3()
{
    int i;
    rtx_dbug_outs((CHAR *)"rtx_test: test3\r\n");
    while (1) 
    {
        printf_0("3\r\n");
        i = g_test_fixture.get_process_priority(3);
        printf_1("Getting priority of pid 3: %i\r\n", i );
        printf_0("Setting priority of pid 3 to 3\r\n");
        g_test_fixture.set_process_priority(3,3);
        printf_1("Getting priority of pid 3: %i\r\n", g_test_fixture.get_process_priority(3));
        printf_1("Setting priority of pid 3 to %i\r\n", i);
        g_test_fixture.set_process_priority(3,i);
        printf_1("Getting priority of pid 3: %i\r\n", g_test_fixture.get_process_priority(3));
        printf_0("Next test should be pid 1\r\n");
   /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
    while (1) 
    {
        printf_0("4\r\n");
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
        printf_0("5\r\n");
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
        printf_0("6\r\n");
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
    
    /*
     * Testing several priority levels
    g_test_proc[0].priority = 1;
    g_test_proc[1].priority = 1;
    g_test_proc[2].priority = 0;
    g_test_proc[3].priority = 2;
    */

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
