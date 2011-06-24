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
#include "../globals.h"
extern void* __end; 

/* third party dummy test process 1 */ 
void test1()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test1\r\n");
    
        int i;
        int failures;
        void* p_mem;
        void* p_mem_array[NUM_MEM_BLKS];


        rtx_dbug_outs((CHAR *) "\r\nBeginning memory tests...\r\n");

        //
        // Attempt to release all memory before allocated (should fail)
        //
        
        failures = 0;
        rtx_dbug_outs((CHAR *) "  Try releasing all memory before being allocated...\r\n");
        for (i=0; i< NUM_MEM_BLKS; i++) {
            int temp;
            temp = release_memory_block( p_mem_array[i] );

            //
            // This should not be successful
            //
            
            if (temp == RTX_SUCCESS) {
                failures++;
            }
        }    

        if (failures > 0) {
            rtx_dbug_outs((CHAR *) "    fail\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "    ok\r\n");
        }

        //
        // Try allocating every block
        // 

        failures = 0;
        rtx_dbug_outs((CHAR *) "  Try allocating every block...\r\n");
        for (i=0; i< NUM_MEM_BLKS; i++) {
            p_mem_array[i] = request_memory_block();
            if (p_mem_array[i] == NULL) {
                rtx_dbug_outs((CHAR *) "    Null pointer.\r\n");
            } else if (p_mem_array[i] > &__end) {
                rtx_dbug_outs((CHAR *) "    Memory out of bound. \r\n");
            }
        }

        if (failures > 0) {
            rtx_dbug_outs((CHAR *) "    fail\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "    ok\r\n");
        }

        // 
        // Try allocating memory after all memory was allocated, this should return NULL
        //
        
        p_mem = request_memory_block();
        if (p_mem == NULL) {
            rtx_dbug_outs((CHAR *) "  Allocate memory block with none available...\r\n    ok\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "  Allocate memory block with none available...\r\n    fail\r\n");
        }

        //
        // Try releasing all memory blocks
        //

        failures = 0;
        rtx_dbug_outs((CHAR *) "  Try releasing all memory blocks...\r\n");
        for (i=0; i< NUM_MEM_BLKS; i++) {
            int temp;
            temp = release_memory_block(p_mem_array[i]);
            if (temp != 0 ) {
                failures++;
            }
        }

        if (failures > 0) {
            rtx_dbug_outs((CHAR *) "    fail\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "    ok\r\n");
        }

        //
        // Attempt to release all memory again (should fail)
        //

        failures = 0;
        rtx_dbug_outs((CHAR *) "  Attempting to release all memory again...\r\n");
        for (i=0; i< NUM_MEM_BLKS; i++) {
            int temp;
            temp = release_memory_block( p_mem_array[i] );
            if (temp == 0 ) {
                failures++;
            }
        }    

        if (failures > 0) {
            rtx_dbug_outs((CHAR *) "    fail\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "    ok\r\n");
        }

        //
        // Allocate all memory, write 128 bytes to each block, deallocate, and then reallocate
        // 
        
        failures = 0;
        rtx_dbug_outs((CHAR *) "  Allocate all memory, write 128 bytes to each block, deallocate, and then reallocate...\r\n");
        for (i=0; i< NUM_MEM_BLKS; i++) {
            p_mem_array[i] = request_memory_block();
            
            int j;        
            unsigned char *current_byte = p_mem_array[i];
            for (j=0; j < 128; j++) {
              *current_byte = 0xFF;
              current_byte++;
            }
        }

        for (i=0; i< NUM_MEM_BLKS; i++) {
            int temp;
            temp = release_memory_block(p_mem_array[i]);
            if (temp == RTX_ERROR) {
                rtx_dbug_outs((CHAR *) "    Failed to deallocate block\r\n");            
                failures++;
            }
        }

        for (i=0; i< NUM_MEM_BLKS; i++) {
            p_mem_array[i] = request_memory_block();

            if (p_mem_array[i] == NULL || p_mem_array[i] > 0x10200000) {
                rtx_dbug_outs((CHAR *) "    Failed to allocate block\r\n");
                failures++;
            }        
        }

        if (failures > 0) {
            rtx_dbug_outs((CHAR *) "    fail\r\n");
        } else {
            rtx_dbug_outs((CHAR *) "    ok\r\n");
        }    

    while (1) 
    {
       /* execute a rtx primitive to test */
            g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
    while (1) 
    {
        /* execute a rtx primitive to test */
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
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
    while (1) 
    {
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
