/**
 * @file: globals.h
 * @brief: holds all of the global variables and settings
 * @date: 06/21/2011
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define NUM_PROCESSES 7
#define NUM_PRIORITIES 4

#define PROCESS_NUM_REGISTERS 15

#define NUM_MEM_BLKS 32
#define MEM_BLK_SIZE 128

void* memory_head;
unsigned long int memory_alloc_field;
void* mem_end;

#endif
