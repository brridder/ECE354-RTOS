/** 
 * @file: rtx.c
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */
 
#include "rtx.h"
#include "globals.h"

int release_processor() {
	int ret_value;
	asm("move.l #0x0, %d0");
	asm("trap #0x0");
	asm("move.l %d0, %0" : "=m" (ret_value));
	return ret_value;	
}

int set_process_priority(int process_ID, int priority) {
    return 0;
}

int get_process_priority(int process_ID) {
    return 0;
}
