/** 
 * @file: rtx.c
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */
 
#include "rtx.h"
#include "dbug.h"
#include "kernel.h"

/**
 * @brief: Performs a system call by calling interrupt vector 0
 * @param: call_id system call id
 * @param: args array of arguments
 * @param: num_args length of args. Maximum is 3.
 */

int do_system_call(int call_id, int** args, int num_args) {
    int return_value;

    //
    // Arguments are passed to the system call via registers.
    // System call ID goes in D0, and the arguments in D1-D3.
    //

    // 
    // Preserve running process' SP
    //
    
    asm("move.l (12, %%sp), %0" : "=r" (running_process->stack));

    asm("move.l %0, %%d0" : : "r" (call_id) : "%%d0");

    /*
    if (num_args > 0) {
        asm("move.l %0, %%d1" : : "m" (args[0]) : "%%d1");
        
        if (num_args > 1) {
            asm("move.l %0, %%d2" : : "m" (args[1]) : "%%d2");

            if (num_args > 2) {
                asm("move.l %0, %%d3" : : "m" (args[2]) : "%%d3");
            }
        }
    }
    */

    asm("trap #0");
    asm("move.l %%d0, %0" : "=r" (return_value));

    return return_value;
}

int release_processor() {
    return do_system_call(0, (void*)0, 0);
}

int set_process_priority(int pid, int priority) {
    return 0;
}

int get_process_priority(int pid) {
    int args[1];
    args[0] = pid;

    return do_system_call(1, &args, 1);
}
