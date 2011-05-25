#include "rtx_inc.h"
#include "dbug.h"
#include "memory.h"
#include "init.h"
#include "globals.h"
#include "init.h"

extern void* __end;

int __main(void) {
    return 0;
}

int main(void) {
    UINT32 stack_start;

    stack_start  = &__end;
    // TODO :: Find top of stack.
    init_processes(stack_start);

    return 0;
}
