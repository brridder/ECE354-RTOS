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
void test1() {
    int i;
    printf_u_0("rtx_test: test1\r\n", 1);

    set_process_priority(1, 1);
    printf_u_1("Getting priority for PID 1: %i\n\r", get_process_priority(1));
    set_process_priority(2, 1);
    printf_u_1("Getting priority for PID 2: %i\n\r", get_process_priority(2));
    
    set_process_priority(1, 3);
    printf_u_1("Getting priority for PID 1: %i\n\r", get_process_priority(1));
    set_process_priority(2, 3);
    printf_u_1("Getting priority for PID 2: %i\n\r", get_process_priority(2));

    i = 0;
    while (1) {
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    printf_u_0("rtx_test: test2\r\n",0);
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 3 */ 
void test3()
{
    printf_u_0("rtx_test: test3\r\n",0);
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    printf_u_0("rtx_test: test4\r\n",0);
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */ 
void test5()
{
    printf_u_0("rtx_test: test5\r\n",0);
    while (1) 
    {
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 6 */ 
void test6()
{
    printf_u_0("rtx_test: test6\r\n",0);
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
        g_test_proc[i].priority = 2;
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
