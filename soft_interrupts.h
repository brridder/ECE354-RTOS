#ifndef _SOFT_INTERRUPTS_H_
#define _SOFT_INTERRUPTS_H_

int do_system_call(int call_id, int** args, int num_args);
void system_call();

#endif
