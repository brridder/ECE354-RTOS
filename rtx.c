/** 
 * @file: rtx.c
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */
 
#include "rtx.h"
#include "dbug.h"
#include "kernel.h"
#include "soft_interrupts.h"

int release_processor() {
    return do_system_call(0, (void*)0, 0);
}

int set_process_priority(int pid, int priority) {
    int args[2];
    args[0] = pid;
    args[1] = priority;
    return do_system_call(2, args, 2);
}

int get_process_priority(int pid) {
    int args[1];
    args[0] = pid;

    return do_system_call(1, args, 1);
}
