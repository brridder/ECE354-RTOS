/** 
 * @file: rtx.h
 * @brief: primatives for rtx
 * @author:
 * @date: 05/27/2011
 */

#ifndef _RTX_H_
#define _RTX_H_

int release_processor();
int set_process_priority(int pid, int priority);
int get_process_priority(int pid);

#endif
