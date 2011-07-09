/**
 * @file: system_processes.c
 * @brief: defines the system processes used by RTX
 * @author: Ben Ridder
 * @date: 05/24/2011
 */

#include "system_processes.h"
#include "../core/process.h"
#include "../rtx.h"
#include "../core/kernel.h"
#include "../core/queues.h"
#include "../lib/dbug.h"
#include "../lib/string.h"
#include "../globals.h"
#include "../uart/uart.h"


char char_out;
int char_handled;

message_queue delayed_messages;
int timer;

/**
 * @brief: null_process that does nothing
 */

void process_null() {
    while (1) {
        rtx_dbug_outs("Null process run.\r\n");

        // TODO :: Remove when we get preemption working
        release_processor();
    }
}

void i_process_uart() {
    uart_interrupt_config inter_cfg;
    message_envelope *message;
    unsigned char uart_state;
    int need_new_line;
    char in_string[64];
    char char_in;
    int i;

    i = 0;
    message = NULL;
    inter_cfg.rx_rdy = false;
    inter_cfg.tx_rdy = true;
    need_new_line = 0; 

    while(1) {
        uart_state = SERIAL1_USR;

        // 
        // Read in waiting data
        //
        
        if (uart_state & 0x01) {
            char_in = SERIAL1_RD;
            
            if (char_in == '\0') {
                continue;
            }

            in_string[i++] = char_in;

            if (char_in == CR) {
                need_new_line = 1;
                char_handled = 1;

                //in_string[i++] = '\n';
                in_string[i++] = '\0';
                
                char_out = CR;
                uart1_set_interrupts(&inter_cfg);

                message = (message_envelope*)request_memory_block();
                message->type = MESSAGE_KEY_INPUT;
                str_cpy(message->data, in_string);
               
                if (in_string[0] == '%') {
                    send_message(KCD_PID, message);
                }
#ifdef _DEBUG_HOTKEYS
                else if (in_string[0] == '!') {
                   uart_debug_decoder(in_string);
                }
#endif

                message = NULL;
                i = 0;
            } else {
                char_out = char_in;
                uart1_set_interrupts(&inter_cfg);
            }
#ifdef UART_DEBUG
            printf_1("uart1 char in : %i\r\n", char_in);
#endif
        //
        // Print out data
        //

        } else if (uart_state & 0x04) {
            SERIAL1_WD = char_out;
            SERIAL1_IMR = 0x02;
            if (char_out == CR) {
                char_out = '\n';
                char_handled = 1;
                uart1_set_interrupts(&inter_cfg);
            } else {
                char_handled = 0;
            }
        }

        release_processor();
    }
}

void i_process_timer() {    
    message_envelope* current_message;
    
    timer = 0;
    delayed_messages.head = NULL;
    delayed_messages.tail = NULL;

    while(1) {
        release_processor();

        //
        // Increment timer
        //
        
        timer++;
        
        #ifdef DEBUG
        if (timer % 1000 == 0) {
            printf_1("Timer: %ims\r\n", timer);
        }
        #endif

        //
        // Check for delayed messages that need to be sent
        //

        if (delayed_messages.head != NULL) {
            while ((timer - delayed_messages.head->delay_start) >=
                   delayed_messages.head->delay) {
                current_message = delayed_messages.head;

                #ifdef DEBUG
                printf_1("Message with delay: %i ready, forwarding\r\n", 
                         current_message->delay);
                #endif
                
                //
                // Remove from queue and deliver
                //

                queue_remove_m(&delayed_messages, current_message);
                k_forward_message(current_message);
            }
        }
    }
}

void process_crt_display() {
    int i;
    int sender_id;
    message_envelope *message;
    uart_interrupt_config int_config;
    
    int_config.tx_rdy = true;
    message = NULL;
    i = 0;
    char_handled = 0;

    while(1) {
        message = receive_message(&sender_id);
        if (message->type == MESSAGE_OUTPUT || 
            message->type == MESSAGE_KEY_INPUT) { 
            i = 0;
            while (message->data[i] != '\0') { 
                if (!char_handled) {
                    char_handled = 1;
                    char_out = message->data[i++];        
                    uart1_set_interrupts(&int_config);
                }
            }
        }

        release_memory_block(message);
        message = NULL;
        
        release_processor();
    }
}

// 
// Keyboard command decoder
//

void process_kcd() {
    int sender_id;
    int num_cmds;
    int i;
    message_envelope *message_receive; 
    message_envelope *message_send; 
    command cmds[32];
    
    num_cmds = 0;

    while(1) {
        message_receive = (message_envelope*)receive_message(&sender_id);
        if (message_receive->type == MESSAGE_KEY_INPUT) {
            for(i = 0; i < num_cmds; i++) {
                if (message_receive->data[1] == cmds[i].cmd_str[1]) {
                    message_send = (message_envelope*)request_memory_block();
                    str_cpy(message_send->data, message_receive->data);
                    send_message(cmds[i].reg_pid, message_send);
#ifdef KCD_DEBUG
                    printf_1("Found it for pid: %i\r\n", cmds[i].reg_pid);
#endif
                    break;
                }
            }
        } else if (message_receive->type == MESSAGE_CMD_REG) {
            str_cpy(cmds[num_cmds].cmd_str, message_receive->data);
            cmds[num_cmds].reg_pid = sender_id;
            num_cmds++;
            release_memory_block(message_receive);
#ifdef KCD_DEBUG
            printf_1("Registered for %i\r\n",  cmds[num_cmds-1].reg_pid);
#endif
        } else {
            release_memory_block(message_receive);
        }

        message_receive = NULL;
    }
}

void process_wall_clock() {
    const unsigned char command[] = "%W";
    char out_string[] = "hh:mm:ss\r\r";
    char digit_buffer[3];
    char* str_iter;
    int sender_id;
    int message_out;
    int clock;
    int clock_display;
    int hours;
    int minutes;
    int seconds;
    message_envelope* message;

    printf_0("Wall clock started\r\n");

    clock = 0;
    clock_display = FALSE;
    message_out = FALSE;

    message = (message_envelope*)request_memory_block();
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, command);
    send_message(KCD_PID, message);

    while (1) {
        if (!message_out) {
            message = (message_envelope*)request_memory_block();
            delayed_send(WALL_CLOCK_PID, message, 1000);
            message_out = TRUE;
        }

        //
        // Try to receive the 1s delayed message sent by this process
        //

        message = receive_message(&sender_id);

        //
        // Update timer display, make sure that the sender of the delayed
        // message was this process.
        //

        if (sender_id == WALL_CLOCK_PID) {
            message_out = FALSE;
            clock++;

            if (clock == 86400) {
                clock = 0;
            }
        
            if (clock_display) {
                seconds = clock % 60;
                minutes = ((clock - seconds) % 3600) / 60;
                hours = ((clock - seconds - (minutes*60)))/3600;
            
                itoa(seconds, digit_buffer);
                if (strlen(digit_buffer) == 1) {
                    out_string[6] = '0';
                    out_string[7] = digit_buffer[0];                
                } else {
                    out_string[6] = digit_buffer[0];
                    out_string[7] = digit_buffer[1];
                }

                itoa(minutes, digit_buffer);
                if (strlen(digit_buffer) == 1) {
                    out_string[3] = '0';
                    out_string[4] = digit_buffer[0];                
                } else {
                    out_string[3] = digit_buffer[0];
                    out_string[4] = digit_buffer[1];
                }

                itoa(hours, digit_buffer);
                if (strlen(digit_buffer) == 1) {
                    out_string[0] = '0';
                    out_string[1] = digit_buffer[0];                
                } else {
                    out_string[0] = digit_buffer[0];
                    out_string[1] = digit_buffer[1];
                }

                printf_u_0(out_string);
            }
        } else if (sender_id == KCD_PID) {
            if (((char*)(message->data))[2] == 'S') {
                
                //
                // Parse time string from %WS HH:mm:ss command
                //
                
                hours = 0;
                minutes = 0;
                seconds = 0;
                str_iter = (char*)(message->data) + 3;
                
                if (consume(&str_iter, ' ') == -1) {
                    printf_0("Parse error: a space must follow command\r\n");
                    goto wall_clock_done;
                } 

                //
                // Skip additional spaces
                //

                while(consume(&str_iter, ' ') == 0) {}

                if (*str_iter == '\r') {
                    printf_0("Parse error: enter a time\r\nx");
                    goto wall_clock_done;
                }

                hours = atoi_e(str_iter, 2);
                if (hours > 23) {
                    printf_1("Bad value for hours: %i\r\n", hours);
                    goto wall_clock_done;
                } else if (hours == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                }
                
                str_iter += 2;
                if (consume(&str_iter, ':') == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                };

                minutes = atoi_e(str_iter, 2);
                if (minutes > 59) {
                    printf_1("Bad value for minutes: %i\r\n", minutes);
                    goto wall_clock_done;
                } else if (minutes == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                }

                str_iter += 2;
                if (consume(&str_iter, ':') == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                };

                seconds = atoi_e(str_iter, 2);
                if (seconds > 59) {
                    printf_1("Bad value for seconds: %i\r\n", seconds);
                    goto wall_clock_done;
                } else if (seconds == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                }

                str_iter += 2;
                if (consume(&str_iter, '\r') == -1) {
                    printf_0("Invalid time string\r\n");
                    goto wall_clock_done;                    
                }
                
                clock = hours * 3600 + minutes * 60 + seconds;
                clock_display = TRUE;
            } else if (((char*)(message->data))[2] == 'T') {

                //
                // Erase the clock
                //
                
                out_string[0] = '\r';
                out_string[1] = ' ';
                out_string[2] = ' ';
                out_string[3] = ' ';
                out_string[4] = ' ';
                out_string[5] = ' ';
                out_string[6] = ' ';
                out_string[7] = ' ';
                out_string[8] = ' ';
                out_string[9] = '\r';
                printf_0(out_string);
                
                out_string[2] = ':';
                out_string[5] = ':';

                clock_display = FALSE;
            }
        }
        
    wall_clock_done:
        release_memory_block(message);
    }
}


void process_set_priority_command() {
    const char *cmd = "%C";
    message_envelope* message; 
    char* str_iter;
    int target_pid;
    int priority;
    int consumed;

    printf_0("Process set priority started\r\n");
    
    //
    // Register for the %C command
    // 

    message = (message_envelope*)request_memory_block();
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, cmd);
    send_message(KCD_PID, message);

    message = 0; 
    target_pid = 0;
    priority = 0;

    while(1) {
        message = (message_envelope*)receive_message((int*)NULL);
        str_iter = (char*)message->data + 2;
        
        while(consume(&str_iter, ' ') == 0);
        
        target_pid = atoi(str_iter, &consumed);
        if (consumed == 0 || target_pid < 0 || target_pid > NUM_PROCESSES-1) {
            printf_0("Invalid PID\r\n");
            goto process_set_priority_command_done;
        }

        str_iter += consumed;

        while(consume(&str_iter, ' ') == 0);
        if (consume(&str_iter, '\r') == 0) {
            printf_0("Priority value expected\r\n");
            goto process_set_priority_command_done;
        }
        
        priority = atoi(str_iter, &consumed);
        if (consumed == 0 || priority < 0 || priority > NUM_PRIORITIES-1) {
            printf_0("Invalid priority\r\n");
            goto process_set_priority_command_done;
        }

        str_iter += consumed;

        while(consume(&str_iter, ' ') == 0);
        if (consume(&str_iter, '\r') == -1) {
            printf_0("Newline expected\r\n");
            goto process_set_priority_command_done;
        }

        set_process_priority(target_pid, priority);

    process_set_priority_command_done:
        release_memory_block((void*)message);
        release_processor();
    }
}

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
            debug_prt_rdy_q();
        }
    } else if (consume(&str, 'b') == 0 || consume(&str, 'B') == 0) {
        if (consume(&str, 'm') == 0 || consume(&str, 'M') == 0) {
            // print blocked memory queues
            debug_prt_blk_mem_q();
        } else if (consume(&str, 'r') == 0|| consume(&str,'R') == 0) {
            // print blocked recevied queues
            debug_prt_blk_rec_q();
        }
    } else { // Bad input
        printf_0("Invalid hot key command for debugging\r\n");
    }
}
