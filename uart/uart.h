#include "../rtx_inc.h"

#ifndef _UART_H_
#define _UART_H_

#include "../rtx_inc.h"

/**
 * UART interrupt configuration
 * @rx_rdy if tru, enables FFULL/RxRDY UART interrupt
 * @tx_rdy if true, enables TxRDY UART interrupt
 */
typedef struct _uart_interrupt_config {
    uint8_t rx_rdy;
    uint8_t tx_rdy;
} uart_interrupt_config;

/**
 * UART Configuration
 * @vector autovector interrupt number
 */
typedef struct _uart_config {
    uint8_t vector;
} uart_config;

/**
 * Set UART1 interrupts
 * @param config interupts to enable
 */
void uart1_set_interrupts(uart_interrupt_config* config);

/**
 * Setup UART1. Baud rate is detected based on _CFSERVER_.
 * @param[in] config UART configuration.
 */
void init_uart1(uart_config* config);

#endif
