/*
 * String.h 
 */

#ifndef _STRING_H_
#define _STRING_H_

void itoa(int n, char* s);
void itox(unsigned int n, char* s);
void reverse(char* s);
int strlen(const char* s);
void string_copy(char* dest, char* src); 
void printf_0(const char* format);
void printf_1(const char* format, int input);

#endif
