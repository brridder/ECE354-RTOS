#ifndef _SYSTEM_PROCESSES_H_
#define _SYSTEM_PROCESSES_H_

void process_null();
void i_process_uart();
void i_process_timer();
void process_crt_display();
void process_kcd();
void process_wall_clock();
void process_set_priority_command();
void uart_debug_decoder();

typedef struct _command {
    char cmd_str[64];
    int reg_pid;
} command;

#endif 
