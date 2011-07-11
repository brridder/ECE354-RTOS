/**
 * @file: globals.h
 * @brief: holds all of the global variables and settings
 * @date: 06/21/2011
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define NUM_PROCESSES 16
#define NUM_PRIORITIES 4

#define PROCESS_NUM_REGISTERS 15

#define NUM_MEM_BLKS 32
#define MEM_BLK_SIZE 128
#define MEM_RESERVED 2

#define UART_READ 0x01
#define UART_WRITE 0x04

#define NUM_KCD_CMDS 32

#define PROC_A 7
#define PROC_B 8
#define PROC_C 9

#define UART_PID 10
#define TIMER_PID 11
#define CRT_DISPLAY_PID 12
#define KCD_PID 13
#define WALL_CLOCK_PID 14
#define SET_PRIORITY_PID 15

#define PROC_A_PRIORITY 2
#define PROC_B_PRIORITY 2
#define PROC_C_PRIORITY 1

#define GID "S11-G031"

#define _DEBUG_HOTKEYS

#endif
