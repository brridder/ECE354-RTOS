#include "../shared/rtx_inc.h"
#include "../shared/string.h"
#include "../shared/uart/uart.h"
#include "../dbug.h"
//#define _DEBUG_

// Globals
SINT32 counter = 0;
unsigned int timer_count = 0;
volatile BYTE char_out = '\0';

int __main(void) {
    return 0;
}

void c_timer_handler(void) {
    timer_count++;
    if (timer_count == 100) {
        //rtx_dbug_outs((char *) "Timer ++ ");
        counter++;
        timer_count = 0;
    }
    TIMER0_TER = 2;
}

void c_serial_handler(void) {
    BYTE temp;
    temp = SERIAL1_USR;
    if(temp & 0x04) {
#ifdef _DEBUG_
        rtx_dbug_outs((char *) "writing data: ");
        rtx_dbug_out_char(char_out);
        rtx_dbug_outs((char*) "\r\n");
#endif /* _DEBUG_ */
        SERIAL1_WD = char_out;
        SERIAL1_IMR = 2;//0x00;
        char_out = '\0';
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
    char out_string[] = "hh:mm:ss\r"; // may need \n\r
    char temp_string[] = "00";
    int i;
    uart_config uart1_config;
    uart_interrupt_config uart1_interrupt_config;
    
    // Disable all interrupts
    asm("move.w #0x2700,%sr");

    coldfire_vbr_init();

    // Store the timer ISR at auto-vector #6
    asm("move.l #asm_timer_entry, %d0");
    asm("move.l %d0, 0x10000078");
    
    // Store the uart1 ISR at user vector #64
    asm("move.l #asm_serial_entry, %d0");
    asm("move.l %d0, 0x10000100");

    // Setup to use auto-vectored interrupt level 6, priority 3
    TIMER0_ICR = 0x9B;

    // Set the reference counts, ~10ms
    TIMER0_TRR = 1758;

    // Setup the timer prescaler and stuff
    TIMER0_TMR = 0xFF1B;

    // Set the interrupt mask
    mask = SIM_IMR;
    mask &= 0x0003ddff;
    SIM_IMR = mask;

    // Setup the UART1 and install handler at autovector interrupt 64
    uart1_config.vector = 64;
    uart1_setup(&uart1_config);

    // Let the timer interrupt fire, lower running priority
    asm("move.w #0x2000, %sr");

    counter = 86390; // 23:59:50

    last_counter = 0; //counter; // So it will print the first time.
    uart1_interrupt_config.tx_rdy = true;
    uart1_interrupt_config.rx_rdy = false;
    i = -1;
    while(1) {
#ifdef _DEBUG_
        rtx_dbug_outs((char*) "loop\r\n");
#endif
        if (last_counter != counter) {
            rtx_dbug_outs((char*) "        \r"); // :S
            // Split up our counter into something more human readable.
            hours = (counter/3600) - (counter % 3600)/3600;
            seconds = counter - hours*3600;
            hours = hours % 24;
            minutes = (seconds)/60 - (seconds%60)/60;
            seconds = seconds - minutes*60;

            // Convert ints to strings;
            itoa(hours, temp_string);
            if (hours < 10) {
                out_string[0] = '0';
                out_string[1] = temp_string[0];
            } else {
                out_string[0] = temp_string[0];
                out_string[1] = temp_string[1];
            }

            itoa(minutes, temp_string);
            if (minutes < 10) {
                out_string[3] = '0';
                out_string[4] = temp_string[0];
            } else { 
                out_string[3] = temp_string[0];
                out_string[4] = temp_string[1];
            } 
            
            itoa(seconds, temp_string);
            if (seconds < 10) {
                out_string[6] = '0';
                out_string[7] = temp_string[0];
            } else { 
                out_string[6] = temp_string[0];
                out_string[7] = temp_string[1];
            } 

            uart1_set_interrupts(&uart1_interrupt_config);
            i = 0;

            last_counter = counter;
        }
        // 
        // Check to see if we have a change in string
        // If i is between the length of the string and the char_out is reset
        // to '\0', load up the next char.
        //
        if( i >= 0 && i < 9 && char_out == '\0') { 
#ifdef _DEBUG_
            rtx_dbug_outs((char*)"char_out");
#endif
            char_out = (char)out_string[i];
            uart1_set_interrupts(&uart1_interrupt_config);
            i++;
        }
    }
    return 0;
}

