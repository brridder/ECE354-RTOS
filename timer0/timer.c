#include "../dbug/dbug.h"
#include "../shared/string.h"
#include "../shared/rtx_inc.h"
#include "../shared/uart/uart.h"

SINT32 counter = 0;
volatile BYTE char_out = '\0';
int __main(void) {
    return 0;
}

void c_timer_handler(void) {
    counter++;
    TIMER0_TER = 2;
}

void c_serial_handler(void) {
    BYTE temp;
    temp = SERIAL1_USR;
    if(temp & 0x04) {
       SERIAL1_WD = char_out;
       SERIAL1_IMR = 2;
    }
}

SINT32 coldfire_vbr_init(void) {
    // Move the VBR into real memory
    asm("move.l %a0, -(%a7)");
    asm("move.l #0x10000000, %a0");
    asm("movec.l %a0, %vbr");
    asm("move.l (%a7)+, %a0");
    return RTX_SUCCESS;
}

int main (void) {
    UINT32 mask;
    SINT32 last_counter;
    int hours, minutes, seconds;
    char out_string[] = "hh:mm:ss"; // may need \n\r
    char temp_string[] = "00";
    int i;
    uart_config uart1_config;
    uart_interrupt_config uart1_interrupt_config;

    asm("move.w #0x2700,%sr");
    coldfire_vbr_init();
    // Store the timer ISR at auto-vector #6
    asm("move.l #asm_timer_entry, %d0");
    asm("move.l %d0, 0x10000078");

    // Setup to use auto-vectored interrupt level 6, priority 3
    TIMER0_ICR = 0x9B;

    // Set the reference counts, ~10ms
    TIMER0_TRR = 1758;

    // Setup the timer prescaler and stuff
    TIMER0_TMR = 0xFF1B;

    // Set the interrupt mask
    mask = SIM_IMR;
    mask &= 0x0003fdff;
    SIM_IMR = mask;

    // Setup the UART1 and install handler at autovector interrupt 64
    uart1_config.vector = 64;
    uart1_setup(&uart1_config);

    // Let the timer interrupt fire, lower running priority
    asm("move.w #0x2000, %sr");

    counter = 86390; // 23:59:50

    last_counter = counter;
    uart1_interrupt_config.tx_rdy = true;
    i = -1;
    while(0) {
        if (last_counter != counter) {
            hours = (counter/3600) - (counter % 3600)/3600;
            seconds = counter - hours*3600;
            minutes = (seconds)/60 - (seconds%60)/60;
            seconds = seconds - minutes*60;
            // TODO :: refactor this.
            itoa(hours, temp_string);

            out_string[0] = temp_string[0];
            out_string[1] = temp_string[1];

            itoa(minutes, temp_string);
            out_string[3] = temp_string[0];
            out_string[4] = temp_string[0];
    
            itoa(seconds, temp_string);
            out_string[6] = temp_string[0];
            out_string[7] = temp_string[0];

            uart1_set_interrupts(&uart1_interrupt_config);
            i = 0;
        }
        if( i >= 0 && i < 8) {
            char_out = out_string[i];
            uart1_set_interrupts(&uart1_interrupt_config);
        }
    }
}

