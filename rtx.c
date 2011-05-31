/** 
 * @file: rtx.c
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */
 
#include "rtx.h"
#include "dbug.h"

int release_processor() {
	int ret_value;

	asm("move.l #0, %d0");
	asm("trap #0");
	asm("move.l %d0, %0" : "=r" (ret_value));
        
	return ret_value;
}

int set_process_priority(int pid, int priority) {
    return 0;
}

int get_process_priority(int pid) {
    int priority;
    
    asm("move.l #1, %d0");
    asm("trap #0");
    asm("move.l %d0, %0" : "=r" (priority));

    return priority;
}
