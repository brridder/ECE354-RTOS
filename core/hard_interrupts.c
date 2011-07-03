#include "hard_interrupts.h"
#include "kernel.h"

void timer_isr() {

    // 
    // In supervisor mode. Disable interrupts.
    //
  
    asm("move.w #0x2700, %sr");

    asm("move.l %a0, -(%sp)"); // A0
    asm("move.l %a1, -(%sp)"); // A1
    asm("move.l %a2, -(%sp)"); // A2
    asm("move.l %a3, -(%sp)"); // A3
    asm("move.l %a4, -(%sp)"); // A4
    asm("move.l %a5, -(%sp)"); // A5
    asm("move.l %a6, -(%sp)"); // A6
    asm("move.l %d0, -(%sp)"); // D0
    asm("move.l %d1, -(%sp)"); // D1
    asm("move.l %d2, -(%sp)"); // D2
    asm("move.l %d3, -(%sp)"); // D3
    asm("move.l %d4, -(%sp)"); // D4
    asm("move.l %d5, -(%sp)"); // D5
    asm("move.l %d6, -(%sp)"); // D6
    asm("move.l %d7, -(%sp)"); // D7

    k_preempt_processor(&processes[TIMER_PID]);
  
    asm("move.l (%sp)+, %d7"); // D7
    asm("move.l (%sp)+, %d6"); // D6
    asm("move.l (%sp)+, %d5"); // D5
    asm("move.l (%sp)+, %d4"); // D4
    asm("move.l (%sp)+, %d3"); // D3
    asm("move.l (%sp)+, %d2"); // D2
    asm("move.l (%sp)+, %d1"); // D1
    asm("move.l (%sp)+, %d0"); // D0
    asm("move.l (%sp)+, %a6"); // A6
    asm("move.l (%sp)+, %a5"); // A5
    asm("move.l (%sp)+, %a4"); // A4
    asm("move.l (%sp)+, %a3"); // A3
    asm("move.l (%sp)+, %a2"); // A2
    asm("move.l (%sp)+, %a1"); // A1
    asm("move.l (%sp)+, %a0"); // A0

    asm("unlk %fp");
    asm("rte");
}

void uart_isr() {
    asm("move.w #0x2700, %sr");

    asm("move.l %a0, -(%sp)"); // A0
    asm("move.l %a1, -(%sp)"); // A1
    asm("move.l %a2, -(%sp)"); // A2
    asm("move.l %a3, -(%sp)"); // A3
    asm("move.l %a4, -(%sp)"); // A4
    asm("move.l %a5, -(%sp)"); // A5
    asm("move.l %a6, -(%sp)"); // A6
    asm("move.l %d0, -(%sp)"); // D0
    asm("move.l %d1, -(%sp)"); // D1
    asm("move.l %d2, -(%sp)"); // D2
    asm("move.l %d3, -(%sp)"); // D3
    asm("move.l %d4, -(%sp)"); // D4
    asm("move.l %d5, -(%sp)"); // D5
    asm("move.l %d6, -(%sp)"); // D6
    asm("move.l %d7, -(%sp)"); // D7
    
    k_preempt_processor(&processes[UART_PID]);
  
    asm("move.l (%sp)+, %d7"); // D7
    asm("move.l (%sp)+, %d6"); // D6
    asm("move.l (%sp)+, %d5"); // D5
    asm("move.l (%sp)+, %d4"); // D4
    asm("move.l (%sp)+, %d3"); // D3
    asm("move.l (%sp)+, %d2"); // D2
    asm("move.l (%sp)+, %d1"); // D1
    asm("move.l (%sp)+, %d0"); // D0
    asm("move.l (%sp)+, %a6"); // A6
    asm("move.l (%sp)+, %a5"); // A5
    asm("move.l (%sp)+, %a4"); // A4
    asm("move.l (%sp)+, %a3"); // A3
    asm("move.l (%sp)+, %a2"); // A2
    asm("move.l (%sp)+, %a1"); // A1
    asm("move.l (%sp)+, %a0"); // A0

    asm("unlk %fp");
    asm("rte");
}
