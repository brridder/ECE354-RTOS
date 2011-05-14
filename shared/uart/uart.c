#include "uart.h"
#include "../rtx_inc.h"

void uart1_set_interrupts(uart_interrupt_config* config) {
    uint8_t mask = 0;
    if (config->rx_rdy) {
        mask |= 0x02;
    } 

    if (config->tx_rdy) {
        mask |= 0x01;
    }

    SERIAL1_IMR = mask;
}

void uart1_setup(uart_config* config) {
    uart_interrupt_config interrupt_config;

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
    SERIAL1_IVR = config->vector;

    // Enable UART1 RX interrupts
    interrupt_config.rx_rdy = true;
    interrupt_config.tx_rdy = false;
    uart1_set_interrupts(&interrupt_config);

    // Set UART1 baud rate
    SERIAL1_UBG1 = 0x00;

    /* Add -D_CFSERVER_ for cf-server build */
    #ifdef _CFSERVER_ 
      SERIAL1_UBG2 = 0x49;  /* cf-server baud rate 19200 */ 
    #else
      SERIAL1_UBG2 = 0x92;  /* lab board baud rate 9600 */
    #endif
    /* _CFSERVER_ */

    // Set UART1 clock mode
    SERIAL1_UCSR = 0xDD;

    // Set UART1 to no parity, 8 bits
    SERIAL1_UMR = 0x13;
    
    // Set UART1 noecho, 1 stop bit
    SERIAL1_UMR = 0x07;
    
    // Setup UART1 tx and rx
    SERIAL1_UCR = 0x05;
}
