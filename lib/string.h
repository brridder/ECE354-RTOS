/*
 * String.h 
 */

#ifndef _STRING_H_
#define _STRING_H_

#include "../rtx.h"

void* memcpy(void* destination, const void* source, int num);
int power(int base, int exponent);
int consume(char** str, const char c);
int atoi_e(char str[], int length);
void itoa(int n, char* s);
void itox(unsigned int n, char* s);
int atoi(const char *s, int* consumed);
void reverse(char* s);
int strlen(const char* s);
void str_cpy(char* dest, char* src); 
int str_cmp(const char* s1, const char* s2);

void printf_0(const char* format);
void printf_1(const char* format, int input);

void printf_u_0(const char* format, int skip_newline);
void printf_u_0_m(const char* format, message_envelope* message);
void printf_u_1(const char* format, int input);
void printf_u_1_m(const char* format, int input, message_envelope* message);

#endif
