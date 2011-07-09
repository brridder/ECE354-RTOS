#include "uart_debug.h"

void uart_debug_decoder(char *str) {
    if (consume(&str, '!') == -1) {
        // ERROR'D
        return;
    }
    
    consume(&str, '!');

    //
    // !RQ == dump out ready queues and priorities
    // !BMQ = dump out blocked memory queues
    // !BRQ = dump out blocked received queues
    //
    
    if (consume(&str,'r') == 0 || consume(&str, 'R') == 0) {
        if (consume(&str,'q') == 0 || consume(&str, 'Q') == 0) {
            // print out ready queues 
            uart_debug_prt_rdy_q();
        }
    } else if (consume(&str, 'b') == 0 || consume(&str, 'B') == 0) {
        if (consume(&str, 'm') == 0 || consume(&str, 'M') == 0) {
            // print blocked memory queues
            uart_debug_prt_blk_mem_q();
        } else if (consume(&str, 'r') == 0|| consume(&str,'R') == 0) {
            // print blocked recevied queues
            uart_debug_prt_blk_rec_q();
        }
    } else { // Bad input
        printf_0("Invalid hot key command for debugging\r\n");
    }
}

void uart_debug_prt_rdy_q() {
    printf_0("Printing ready queues and priorities\r\n");
}

void uart_debug_prt_blk_mem_q() {
    printf_0("Printing blocked memory queues\r\n");
}

void uart_debug_prt_blk_rec_q() {
    printf_0("Printing blocked received queues\r\n");
}

