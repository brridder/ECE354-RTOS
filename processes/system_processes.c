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

//#define KCD_DEBUG

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
            send_message(CRT_DISPLAY_PID, message_receive);
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
        release_processor();
    }
}

void process_wall_clock() {
    const unsigned char command[] = "%W";
    char out_string[] = "hh:mm:ss\r";
    char digit_buffer[3];
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
            message = request_memory_block();
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

                printf_0(out_string);
            }
        } else if (sender_id == KCD_PID) {
            if (((char*)(message->data))[2] == 'S') {

                //
                // TODO: Check time validity
                // 

                //
                // TODO: Set the time
                //

                clock_display = TRUE;
            } else if (((char*)(message->data))[2] == 'T') {
                clock_display = FALSE;
            }
        }

        release_memory_block(message);
    }
}


void process_set_priority_command() {
    const char *cmd = "%C";
    message_envelope* message; 
    char in_str[100];
    char str_buf[50];

    int target_pid;
    int priority;
    int i;
    int j;

    // Register the fucking command
    message = (message_envelope*)request_memory_block();
    message->type = MESSAGE_CMD_REG;
    str_cpy(message->data, cmd);
    send_message(KCD_PID, message);
    message = 0; 
    target_pid = 0;
    priority = 0;

    printf_0("Process set priority started\r\n");
    while(1) {
        message = (message_envelope*)receive_message(0);
        //while(message->type != MESSAGE_KEY_INPUT || message == 0) {
        //}
        str_cpy(in_str, message->data);        
        
        i = 0;
        while(in_str[i++] != ' ');
        
        j = 0;
        while(in_str[i] != ' ') {
            str_buf[j] = in_str[i];
            i++;
        }
        
        target_pid = atoi(str_buf);
        
        j = 0;
        while(in_str[i] != '\r') {
            str_buf[j] = in_str[i];
            i++;
        }
        priority = atoi(str_buf);
        set_process_priority(target_pid, priority);
        release_processor();
    }
}
