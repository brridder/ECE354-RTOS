/**
 * @file: serial.c
 * @brief: UART1/second serial port/rtx terminal I/O sample code
 * @author: Casey Banner & David Janssen
 * @date: 2011/05/13
 */
#include "../shared/rtx_inc.h"
#include "../dbug/dbug.h"

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

SINT32 coldfire_vbr_init( VOID )
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
int main( void )
{
    UINT32 mask;

    // Disable all interrupts
    asm( "move.w #0x2700,%sr" );
    
    coldfire_vbr_init();
    
    /*
     * Store the serial ISR at user vector #64
     */
    asm( "move.l #asm_serial_entry,%d0" );
    asm( "move.l %d0,0x10000100" );

    // Reset UART1
    SERIAL1_UCR = 0x10;

    // Reset UART1 receiver
    SERIAL1_UCR = 0x20;

    // Reset UART1 transmitter
    SERIAL1_UCR = 0x30;
    
    // Reset UART1 error condition
    SERIAL1_UCR = 0x40;

    // Install UART1 interrupt
    SERIAL1_ICR = 0x17;
    SERIAL1_IVR = 64;

    // Enable UART1 RX interrupts
    SERIAL1_IMR = 0x02;

    // Set UART1 baud rate
    SERIAL1_UBG1 = 0x00;
#ifdef _CFSERVER_           /* add -D_CFSERVER_ for cf-server build */
    SERIAL1_UBG2 = 0x49;    /* cf-server baud rate 19200 */ 
#else
    SERIAL1_UBG2 = 0x92;    /* lab board baud rate 9600 */
#endif /* _CFSERVER_ */

    // Set UART1 clock mode
    SERIAL1_UCSR = 0xDD;

    // Set UART1 to no parity, 8 bits
    SERIAL1_UMR = 0x13;
    
    // Set UART1 noecho, 1 stop bit
    SERIAL1_UMR = 0x07;
    
    // Setup UART1 tx and rx
    SERIAL1_UCR = 0x05;

    // Enable interupts
    mask = SIM_IMR;
    mask &= 0x0003dfff;
    SIM_IMR = mask;
    
    // Enable all interupts
    asm( "move.w #0x2000,%sr" );

    rtx_dbug_outs((CHAR *) "Type Q or q on RTX terminal to quit.\n\r" );
    
    /* Busy Loop */
    while( char_in != 'q' && char_in != 'Q' )
    {
        if(!char_handled)
        {
            char_handled = TRUE;
            char_out = char_in;
            
            /* Nasty hack to get a dynamic string format, 
             * grab the character before turning the interrupts back on. 
             */
            string_format[12] = char_in;

            /* enable tx interrupts  */
            SERIAL1_IMR = 3;

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
