#include "soft_interrupts.h"

#include "kernel.h"
#include "dbug.h"

void system_call() {
    int call_id;

    rtx_dbug_outs("System call");

    // 
    // In supervisor mode
    //

    asm("move.w #0x2700,%sr");
	
    asm("move.l %a0, -(%a7)");
    asm("move.l %a1, -(%a7)");
    asm("move.l %a2, -(%a7)");
    asm("move.l %a3, -(%a7)");
    asm("move.l %a4, -(%a7)");
    asm("move.l %a5, -(%a7)");
    asm("move.l %a6, -(%a7)");
    asm("move.l %d0, -(%a7)");
    asm("move.l %d1, -(%a7)");
    asm("move.l %d2, -(%a7)");
    asm("move.l %d3, -(%a7)");
    asm("move.l %d4, -(%a7)");
    asm("move.l %d5, -(%a7)");
    asm("move.l %d6, -(%a7)");
    asm("move.l %d7, -(%a7)");
	
    asm("move.l %d0, %0" : "=m" (call_id));
	
    switch(call_id) {

        //
        // 0 : release_processor()
        // 

        case 0: 
            k_release_processor();
            break;
        default:
           // TODO: Handle this case
           break;
    }
	
    asm("move.l (%a7)+, %d7");
    asm("move.l (%a7)+, %d6");
    asm("move.l (%a7)+, %d5");
    asm("move.l (%a7)+, %d4");
    asm("move.l (%a7)+, %d3");
    asm("move.l (%a7)+, %d2");
    asm("move.l (%a7)+, %d1");
    asm("move.l (%a7)+, %d0");
    asm("move.l (%a7)+, %a6");
    asm("move.l (%a7)+, %a5");
    asm("move.l (%a7)+, %a4");
    asm("move.l (%a7)+, %a3");
    asm("move.l (%a7)+, %a2");
    asm("move.l (%a7)+, %a1");
    asm("move.l (%a7)+, %a0");
	
    asm("rte");
}
