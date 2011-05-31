/**
 * @file: serial.c
 * @brief: UART1/second serial port/rtx terminal I/O sample code
 * @author: Casey Banner & David Janssen
 * @date: 2011/05/13
 */
#include "../shared/rtx_inc.h"
#include "../shared/uart/uart.h"
#include "../dbug.h"

/*
 * Global Variables
 */
volatile BYTE char_in = '!';
volatile BOOLEAN char_handled = TRUE;
volatile BYTE char_out = '\0';
CHAR string_format[] = "You Typed a Q\n\r";

int __main(void)
{
    return 0;
}

VOID c_serial_handler( VOID )
{
    BYTE temp;

    temp = SERIAL1_USR;    /* Ack the interrupt */

#ifdef _DEBUG_
    rtx_dbug_outs((CHAR *) "Enter: c_serial_handler ");
#endif /* _DEBUG_ */
    
    /* See if data is waiting.... */    
    if( temp & 0x01 ) {
#ifdef _DEBUG_
        rtx_dbug_outs((CHAR *) "reading data: ");
#endif /* _DEBUG_ */

        char_in = SERIAL1_RD;
        char_handled = FALSE;

#ifdef _DEBUG_
        rtx_dbug_out_char(char_in);
    	rtx_dbug_outs((CHAR *) "\r\n");
#endif /* _DEBUG_ */
    }

    /* See if port is ready to accept data */    
    else if ( temp & 0x04 ) {
#ifdef _DEBUG_
        rtx_dbug_outs((CHAR *) "writing data: ");
        rtx_dbug_out_char(char_out);
    	rtx_dbug_outs((CHAR *) "\r\n");
#endif /* _DEBUG_*/

        if (char_out == '\r') {
          char_handled = FALSE;
          char_in = '\n';
        }

        SERIAL1_WD = char_out;   /* Write data to port */
        SERIAL1_IMR = 2;        /* Disable tx Interupt */
    }

    return;
}

SINT32 coldfire_vbr_init(void)
{
    asm( "move.l %a0, -(%a7)" );
    asm( "move.l #0x10000000, %a0 " );
    asm( "movec.l %a0, %vbr" );
    asm( "move.l (%a7)+, %a0" );
    
    return RTX_SUCCESS;
}

/*
 * Entry point, check with m68k-coff-nm
 */
int main(void) {
    UINT32 mask;
    uart_config uart1_config;
    uart_interrupt_config uart1_interrupt_config;

    // Disable all interrupts
    asm("move.w #0x2700,%sr");
    
    coldfire_vbr_init();

    // Store the serial ISR at user vector #64 
    asm("move.l #asm_serial_entry,%d0");
    asm("move.l %d0,0x10000100");
    
    // Setup UART1 and install handler at autovector interrupt 64
    uart1_config.vector = 64;
    uart1_setup(&uart1_config);

    // Enable interupts
    mask = SIM_IMR;
    mask &= 0x0003dfff;
    SIM_IMR = mask;
    
    // Enable all interupts
    asm("move.w #0x2000,%sr");

    rtx_dbug_outs((CHAR *) "Type Q or q on RTX terminal to quit.\n\r" );
    
    uart1_interrupt_config.tx_rdy = true;
    while(char_in != 'q' && char_in != 'Q') {
        if(!char_handled) {
            char_handled = TRUE;
            char_out = char_in;
            
            /* Nasty hack to get a dynamic string format, 
             * grab the character before turning the interrupts back on. 
             */
            string_format[12] = char_in;

            /* enable tx interrupts  */
            uart1_set_interrupts(&uart1_interrupt_config);

            /* Now print the string to debug, 
             * note that interrupts are now back on. 
             */
            rtx_dbug_outs(string_format);
        }
    }

    /* Disable all interupts */
    asm( "move.w #0x2700,%sr" );

    /* Reset globals so we can run again */
    char_in = '\0';
    char_handled = TRUE;
    return 0;
}
