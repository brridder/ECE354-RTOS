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
int uart_skip_newline;

message_queue delayed_messages;
int timer;

/**
 * @brief: null_process that does nothing
 */

void process_null() {
    while (1) {
        rtx_dbug_outs("Null process run.\r\n");
        release_processor();
    }
}

/**
 * i_process_uart: Interrupt process for uart interrupt requests.
 *
 * Does NOT use the interprocess communication system for printing to the
 * serial port as this WILL block this process which is bad.
 */

void i_process_uart() {
    uart_interrupt_config inter_cfg;
    message_envelope *message;
    unsigned char uart_state;
    char in_string[64];
    char char_in;
    int i;

    i = 0;
    message = NULL;
    
    //
    // Prepare the interrupt config struct to prevent rx and turn on tx.
    //

    inter_cfg.rx_rdy = false;
    inter_cfg.tx_rdy = true;
    uart_skip_newline = 0;

    while(1) {

        //
        // Get UART state
        //
        
        uart_state = SERIAL1_USR;

        // 
        // Read in waiting data if uart_state == UART_READ
        //
        
        if (uart_state & UART_READ) {
            //
            // Get char from serial buffer
            //
            
            char_in = SERIAL1_RD;
            
            //
            // If input is NUL, go back to the start of loop
            //
            
            if (char_in == '\0') {
                continue;
            }
            
            //
            // Store in character in the string buffer
            //

            in_string[i++] = char_in;

            //
            // Enter was pressed, so send the input to the appropriate location
            //
            
            if (char_in == CR) {
                char_handled = 1;

                //
                // NUL terminate the string
                //
                
                in_string[i++] = '\0';
                
                //
                // Output the CR and don't skip the new line on the echo
                //
                
                char_out = CR;
                uart_skip_newline = 0;
                uart1_set_interrupts(&inter_cfg);
                
                //
                // Started with %, so pass it onto the KCD process
                //

                if (in_string[0] == '%') {
                    message = (message_envelope*)request_memory_block();
                    message->type = MESSAGE_KEY_INPUT;
                    str_cpy(message->data, in_string);
                    send_message(KCD_PID, message);
                }
#ifdef _DEBUG_HOTKEYS
                //
                // Started with !, so go into the debugging decoder 
                //
                else if (in_string[0] == '!') {
                   uart_debug_decoder(in_string);
                }
#endif
                // 
                // Reset buffers and pointers
                // 
                message = NULL;
                i = 0;
            } else {

                //
                // Echo the input
                //
                
                char_out = char_in;
                uart1_set_interrupts(&inter_cfg);
            }

#ifdef UART_DEBUG
            printf_1("uart1 char in : %i\r\n", char_in);
#endif
        //
        // Print out data if uart_state == UART_WRITE
        //

        } else if (uart_state & UART_WRITE) {
            //
            // Write out char to the write register
            // Ack the interrupt
            //
            SERIAL1_WD = char_out;
            SERIAL1_IMR = 0x02;
            
            //
            // Two cases to handle: One to output a new line, another to skip
            // it
            //

            if (char_out == CR && !uart_skip_newline) {
                char_out = '\n';
                char_handled = 1;
                uart1_set_interrupts(&inter_cfg);
            } else {
                char_handled = 0;
                uart_skip_newline = 0;
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
        
        #ifdef _DEBUG
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

                #ifdef _DEBUG
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

/**
 * process_crt_display: receives messages from other processes and handles the
 * printing to the uart using the uart ISR and i_process_uart
 *
 * Does NOT use the IPC for sending a single character to the uart as this WILL
 * block the uart which is bad.
 */

void process_crt_display() {
    int i;
    int sender_id;
    message_envelope *message;
    uart_interrupt_config int_config;
    
    int_config.tx_rdy = true;
    int_config.rx_rdy = false;
    message = NULL;
    i = 0;

    while(1) {
        //
        // Get a message to print
        //

        message = receive_message(&sender_id);

        //
        // Verify message is of the correct type.
        //
        
        if (message->type == MESSAGE_OUTPUT || 
            message->type == MESSAGE_OUTPUT_NO_NEWLINE ||
            message->type == MESSAGE_KEY_INPUT ) { 
            i = 0;
            
            //
            // Iterate over the string until we get to the end (NULL) 
            //
            
            while (message->data[i] != '\0') { 

                //
                // Only move the next char into char_out buffer if the process
                // is ready.
                //
                
                if (!char_handled) {
                    char_handled = 1;

                    // 
                    // Skip the new line if CR and correct message, useful for
                    // the wall clock process to override itself
                    //
                    
                    if (message->data[i] == CR && 
                        message->type == MESSAGE_OUTPUT_NO_NEWLINE) {
                        uart_skip_newline = 1; 
                    }
                    
                    // 
                    // Move the next char into the buffer, increment our
                    // iterator, and signal to the UART ISR to fire
                    //

                    char_out = message->data[i++];        
                    uart1_set_interrupts(&int_config);
                }
            }
        }
        
        //
        // Free up our memory received by the message
        //

        release_memory_block(message);
        message = NULL;
        
        release_processor();
    }
}

/** 
 * Keyboard command decoder: Takes input from the uart and sends it to the
 * correct, registered process.
 */

void process_kcd() {
    int sender_id;
    int num_cmds;
    int i;
    message_envelope *message;
    command cmds[NUM_KCD_CMDS];
    
    num_cmds = 0;

    while(1) {

        //
        // Get a message
        //

        message = (message_envelope*)receive_message(&sender_id);

        //
        // It is a message to pass to another process
        //
        
        if (message->type == MESSAGE_KEY_INPUT) {

            //
            // Iterate over the cmds list and try to find it.
            //
            
            for(i = 0; i < num_cmds; i++) {

                //
                // Only need to compare the first character from our
                // understanding of the requirements. If found, forward the
                // message to the register
                //
                
                if (message->data[1] == cmds[i].cmd_str[1]) {
                    send_message(cmds[i].reg_pid, message);
#ifdef KCD_DEBUG
                    printf_1("Found it for pid: %i\r\n", cmds[i].reg_pid);
#endif
                    break;
                }
            }

            // 
            // Didn't find it, release the memory alloc'd by the sender
            //
            
            if (i == num_cmds) {
                release_memory_block(message);
            }
        
        //
        // It is a message to register a command for a process (sender). Copy
        // the cmd string into the cmd data structure and save the register's
        // pid, increment the total number of commands, release memory
        //

        } else if (message->type == MESSAGE_CMD_REG) {
            str_cpy(cmds[num_cmds].cmd_str, message->data);
            cmds[num_cmds].reg_pid = sender_id;
            num_cmds++;
            release_memory_block(message);
#ifdef KCD_DEBUG
            printf_1("Registered for %i\r\n",  cmds[num_cmds-1].reg_pid);
#endif
        //
        // Received message was the incorrect type, just dealloc it.
        //
        
        } else {
            release_memory_block(message);
        }

        message = NULL;
    }
}

void process_wall_clock() {
    const unsigned char command[] = "%W";
    char out_string[] = "hh:mm:ss \r";
    char digit_buffer[3];
    char* str_iter;
    int sender_id;

    int sent_message;

    int clock;
    int clock_display;
    int hours;
    int minutes;
    int seconds;

    message_envelope* message;
    message_envelope* message_delayed;

    message_delayed = NULL;
    sent_message = FALSE;

#ifdef _DEBUG
    printf_0("Wall clock started\r\n");
#endif 
    
    clock = 0;
    clock_display = FALSE;

    message = (message_envelope*)request_memory_block();
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, command);
    send_message(KCD_PID, message);

    while (1) {
        
        //
        // If we currently aren't waiting for a delayed message, send one
        // to ourself with a delay of 1 second.
        //

        if (!sent_message) {
            if (message_delayed == NULL) {
                message_delayed = (message_envelope*)request_memory_block();
            }

            delayed_send(WALL_CLOCK_PID, message_delayed, 1000);
            sent_message = TRUE;
        }

        //
        // Try to receive either the 1 second delayed wall clock message,
        // or a command from the KCD.
        //

        message = receive_message(&sender_id);
        if (sender_id == WALL_CLOCK_PID) {
            sent_message = FALSE;
            clock++;

            //
            // Roll the clock over after 24 hours
            //

            if (clock == 86400) {
                clock = 0;
            }
        
            if (clock_display) {

                //
                // Update clock output string
                //

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

                //
                // Send clock output to CRT, reusing the memory block that we 
                // received from delayed_send
                //

                message->type = MESSAGE_OUTPUT_NO_NEWLINE;
                str_cpy(message->data, out_string);
                send_message(CRT_DISPLAY_PID, message);
                message_delayed = NULL;
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
                    printf_u_0_m("Parse error: a space must follow command\r\n",
                                 message);
                    message = NULL;
                    goto wall_clock_done;
                } 

                //
                // Skip additional spaces
                //

                while(consume(&str_iter, ' ') == 0) {}

                if (*str_iter == '\r') {
                    printf_u_0_m("Parse error: enter a time\r\n", message);
                    message = NULL;
                    goto wall_clock_done;
                }

                hours = atoi_e(str_iter, 2);
                if (hours > 23) {
                    printf_u_1_m("Bad value for hours: %i\r\n", hours, message);
                    message = NULL;
                    goto wall_clock_done;
                } else if (hours == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
                    goto wall_clock_done;                    
                }
                
                str_iter += 2;
                if (consume(&str_iter, ':') == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
                    goto wall_clock_done;                    
                };

                minutes = atoi_e(str_iter, 2);
                if (minutes > 59) {
                    printf_u_1_m("Bad value for minutes: %i\r\n", minutes, message);
                    message = NULL;
                    goto wall_clock_done;
                } else if (minutes == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
                    goto wall_clock_done;                    
                }

                str_iter += 2;
                if (consume(&str_iter, ':') == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
                    goto wall_clock_done;                    
                };

                seconds = atoi_e(str_iter, 2);
                if (seconds > 59) {
                    printf_u_1_m("Bad value for seconds: %i\r\n", seconds, message);
                    message = NULL;
                    goto wall_clock_done;
                } else if (seconds == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
                    goto wall_clock_done;                    
                }

                str_iter += 2;
                if (consume(&str_iter, '\r') == -1) {
                    printf_u_0_m("Invalid time string\r\n", message);
                    message = NULL;
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
                
                printf_u_0_m(out_string, message);
                message = NULL;
                
                out_string[2] = ':';
                out_string[5] = ':';

                clock_display = FALSE;
            }

            //
            // We are done with this message, release it
            //

        wall_clock_done:
            if (message != NULL) {
                release_memory_block(message);
            }

        } else {            

            //
            // Unknown message type received, release it
            //

            release_memory_block(message);
        }
    }
}


void process_set_priority_command() {
    const char *cmd = "%C";
    message_envelope* message; 
    char* str_iter;
    int target_pid;
    int priority;
    int consumed;

#ifdef _DEBUG
    printf_0("Process set priority started\r\n");
#endif

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
        
        //
        // Get a message, don't care about the sender
        //
        
        message = (message_envelope*)receive_message((int*)NULL);

        //
        // Skip the first two characters, %C, and start at first ' '
        //
        
        str_iter = (char*)message->data + 2;
        
        // 
        // Strip out whitespace
        //
        
        while(consume(&str_iter, ' ') == 0);
       
        //
        // Get the target's PID and ensure it is valid
        //
        
        target_pid = atoi(str_iter, &consumed);
        if (consumed == 0 || target_pid < 0 || target_pid > NUM_PROCESSES-1) {
            printf_u_0_m("Invalid PID\r\n", message);
            goto process_set_priority_command_done;
        }
        
        str_iter += consumed;
        
        while(consume(&str_iter, ' ') == 0);

        // 
        // Check to ensure that input has a priority value
        //

        if (consume(&str_iter, '\r') == 0) {
            printf_u_0_m("Priority value expected\r\n", message);
            goto process_set_priority_command_done;
        }
       
        //
        // Get the new priority value and ensure validity
        //
        
        priority = atoi(str_iter, &consumed);
        if (consumed == 0 || priority < 0 || priority > NUM_PRIORITIES-1) {
            printf_u_0_m("Invalid priority\r\n", message);
            goto process_set_priority_command_done;
        }

        str_iter += consumed;
        
        while(consume(&str_iter, ' ') == 0);

        //
        // If the line wasn't terminated correctly, error'd
        //

        if (consume(&str_iter, '\r') == -1) {
            printf_u_0_m("Newline expected\r\n", message);
            goto process_set_priority_command_done;
        }

        //
        // Kernel function call to set priority of the target as parsed
        //

        set_process_priority(target_pid, priority);\

        // 
        // Release the received memory block
        //
        
        release_memory_block((void*)message);

    process_set_priority_command_done:
        release_processor();
    }
}

/**
 * uart_debug_decoder: Used to decode and select the appropriate debug function
 * call 
 */

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
    // !FM == dump out # of free memory blocks
    // !M == dump out last 10 sent and recieved messages
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
    } else if (consume(&str, 'f') == 0 || consume(&str, 'F') == 0) {
        if (consume(&str, 'm') == 0 || consume(&str, 'M') == 0) {
            // print number of memory blocks free
            debug_prt_mem_blks_free();
        }
    } else if (consume(&str, 'm') == 0 || consume(&str, 'M') == 0) {
        debug_prt_message_history();
    } else { // Bad input
        printf_0("Invalid hotkey command for debugging\r\n");
        printf_0("The following are valid hotkeys:\r\n");
        printf_0("    !RQ:  Print ready processes and priorities\r\n");
        printf_0("    !BMQ: Print out processes blocked on memory\r\n");
        printf_0("    !BRQ: Print out processes blocked on receiving messages\r\n");
        printf_0("    !FM:  Print out the # of free memory blocks\r\n");
        printf_1("    !M:   Print out the last %i sent", DEBUG_MESSAGE_LOG_SIZE);
        printf_1("and %i received messages\r\n", DEBUG_MESSAGE_LOG_SIZE);
    }
}
