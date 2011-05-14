/**
 * @author: 
 * @brief: ECE 354 S11 RTX Project P1-(c) 
 * @date: 2011/05/02
 */

#include "rtx_inc.h"
#include "dbug.h"
#include "shared/string.h"
#include "memory.h"

#define NUM_MEM_BLKS 32
#define BLOCK_SIZE 128

//
// Start of free memory
//

extern void* __end;

//
// Head pointer of the start of the free list
//

void *memory_head;

//
// Field to keep track of allocated memory blocks
//

UINT32 memory_alloc_field;


/**
 * @brief calculate the index for a particular block
 * Internal function used to set bits in the memory 
 * allocation field.
 */ 
int get_block_index(void* addr) {
    //
    // The block index is its offset into the free memory region (in bytes)
    // divided by the block size.
    //
 
    return ((int)addr - (int)&__end) / BLOCK_SIZE;
}

/**
 * @brief memory management initialization routine
 */
void init_memory() {
    int i;
    int *current_block;
    
    //
    // The head pointer starts at the start of free memory plus
    // the space required for all our memory blocks (32 blocks * 128 bytes).
    // Since this is pointer arithmetic, adding 1 adds 4 bytes.
    //

    memory_head = &__end + ((NUM_MEM_BLKS-1)*NUM_MEM_BLKS);

    //
    // Iterate through the memory pool and setup the free list.
    // The first 4 bytes of each memory block contain the address
    // of the next free memory block.
    //
    // When decrementing the pointer, block_size/4 must be used (pointer math).
    // When decrementing the valud pointed to by the iterator, block_size 
    // can be used (integer math).
    //
 
    current_block = (int*)memory_head + BLOCK_SIZE/sizeof(void*);
    for (i = 0; i < NUM_MEM_BLKS; i++) {
        current_block -= BLOCK_SIZE/sizeof(void*);
        *current_block = (int)current_block - BLOCK_SIZE;
    }
    *current_block = NULL;

    //
    // Setup the memory allocation field. Each bit in this field
    // represents one block in the pool. A value of 0 means 
    // the block has not been allocated, 1 means the block has been allocated.
    //
    
    memory_alloc_field = 0;
}

/**
 * @brief request a free memory block of size 128 Bytes
 * @return starting address of a free memory block
 *         and NULL on error 
 */

void* s_request_memory_block() {   
    int block_index;
    void* block;
    
    //
    // Check if we have any free memory left. If not, return NULL
    //

    if (memory_head != NULL) {
        //
        // Allocate the block on the top of the free list, moving
        // the head of the free list to the next available block
        //

        block = memory_head;
        memory_head = (void*)*(UINT32*)block;

        //
        // Set the bit in the memory field corresponding to this block
        // 

        block_index = get_block_index(block);
        memory_alloc_field |= (0x01 << block_index);

        return block;
    }
    
    return NULL;
}

/**
 * @param: address of a memory block to free
 * @return: 0 on sucess, non-zero on error
 */

int s_release_memory_block(void* memory_block)
{
    int block_index;

    //
    // Check the memory allocation field to see if this block has already
    // been deallocated.
    //
    
    block_index = get_block_index(memory_block);
    if (memory_alloc_field & (0x01 << block_index)) {
      *(int*)memory_block = (int)memory_head;
      memory_head = memory_block;

      //
      // Update the allocated memory field
      //

      memory_alloc_field &= ((0x01 << block_index) ^ 0xFFFFFFFF);

      //
      // Success
      //

      return RTX_SUCCESS;
    }

    //
    // This memory block is not currently allocated, failure
    //
    
    return RTX_ERROR;
}
