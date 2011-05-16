/**
 * @author: Casey Banner
 * @brief: ECE 354 S11 RTX Project P1-(c) 
 * @date: 2011/05/02
 */

#include "../rtx_inc.h"
#include "../dbug.h"
#include "../memory.h"

#define NUM_MEM_BLKS 32

int __main( VOID )
{
    return 0;
}

int main( VOID )
{
    int i;
    int failures;
    void* p_mem;
    void* p_mem_array[NUM_MEM_BLKS];

    init_memory();

    rtx_dbug_outs((CHAR *) "\r\nBeginning memory tests...\r\n");

    //
    // Attempt to release all memory before allocated (should fail)
    //
    
    failures = 0;
    rtx_dbug_outs((CHAR *) "  Try releasing all memory before being allocated...\r\n");
    for (i=0; i< NUM_MEM_BLKS; i++) {
        int temp;
        temp = s_release_memory_block( p_mem_array[i] );

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
        p_mem_array[i] = s_request_memory_block();
        if (p_mem_array[i] == NULL) {
            rtx_dbug_outs((CHAR *) "    Null pointer.\r\n");
        } else if (p_mem_array[i] > 0x10200000) {
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
    
    p_mem = s_request_memory_block();
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
        temp = s_release_memory_block(p_mem_array[i]);
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
        temp = s_release_memory_block( p_mem_array[i] );
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
        p_mem_array[i] = s_request_memory_block();
        
        int j;        
        unsigned char *current_byte = p_mem_array[i];
        for (j=0; j < 128; j++) {
          *current_byte = 0xFF;
          current_byte++;
        }
    }

    for (i=0; i< NUM_MEM_BLKS; i++) {
        int temp;
        temp = s_release_memory_block(p_mem_array[i]);
        if (temp == RTX_ERROR) {
            rtx_dbug_outs((CHAR *) "    Failed to deallocate block\r\n");            
            failures++;
        }
    }

    for (i=0; i< NUM_MEM_BLKS; i++) {
        p_mem_array[i] = s_request_memory_block();

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

    return 0;
}
