#include "rtx_test.h"
#include "../rtx_inc.h"
#include "../lib/dbug.h"
#include "../lib/string.h"

/* third party dummy test process 1 */ 
void test1()
{
    int t;
    int i;
    int total;
    int delta;    
    int* timer = g_profiler.timer;
    void* block;
    
    TIMER1_ICR = 0x98;
    TIMER1_TRR = 0xFFFFFFFF;
    TIMER1_TMR = 0xF923;

    total = 0;
    for (i = 0; i < 1000; i++) {
        t = TIMER1_TCN;
        block = g_test_fixture.request_memory_block();
        delta = TIMER1_TCN - t;
        total += delta;
        TIMER1_TCN = 0;

        if (delta < 0) {
            printf_0("Error: negative delta\r\n");
        }

        g_test_fixture.release_memory_block(block);
    }

    printf_1("Time for request_memory_block() (avg): %i\r\n", total/1000);

    total = 0;
    for (i = 0; i < 1000; i++) {
        block = g_test_fixture.request_memory_block();

        t = TIMER1_TCN;
        g_test_fixture.release_memory_block(block);
        delta = TIMER1_TCN - t;
        total += delta;
        TIMER1_TCN = 0;

        if (delta < 0) {
            printf_0("Error: negative delta\r\n");
        }
    }

    printf_1("Time for release_memory_block() (avg): %i\r\n", total/1000);

    total = 0;
    for (i = 0; i < 1000; i++) {
        block = g_test_fixture.request_memory_block();

        t = TIMER1_TCN;
        g_test_fixture.send_message(1, block);
        delta = TIMER1_TCN - t;
        total += delta;
        TIMER1_TCN = 0;

        if (delta < 0) {
            printf_0("Error: negative delta\r\n");
        }

        block = g_test_fixture.receive_message(NULL);
        g_test_fixture.release_memory_block(block);
    }

    printf_1("Time for send_message() (avg): %i\r\n", total/1000);

    total = 0;
    for (i = 0; i < 1000; i++) {
        block = g_test_fixture.request_memory_block();
        g_test_fixture.send_message(1, block);

        t = TIMER1_TCN;
        block = g_test_fixture.receive_message(NULL);
        delta = TIMER1_TCN - t;
        total += delta;
        TIMER1_TCN = 0;

        if (delta < 0) {
            printf_0("Error: negative delta\r\n");
        }

        g_test_fixture.release_memory_block(block);
    }

    printf_1("Time for receive_message() (avg): %i\r\n", total/1000);


    while (1) 
    {
        //printf_1("Timer: %i\r\n", *timer);
        //printf_1("Timer: %i\r\n", TIMER1_TCN);
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    while (1) 
    {
        //printf_0("2\r\n");
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 3 */ 
void test3()
{
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

int main(void)
{
    return 0;
}
