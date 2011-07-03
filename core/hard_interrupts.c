#include "hard_interrupts.h"
#include "kernel.h"

void timer_isr() {
  asm("unlk %fp");
  asm("rte");
}