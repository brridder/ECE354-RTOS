/*
 * String.h 
 */

#ifndef _STRING_H_
#define _STRING_H_

int power(int base, int exponent);
int consume(char** str, const char c);
int atoi(char str[], int length);
void itoa(int n, char* s);
void itox(unsigned int n, char* s);
void reverse(char* s);
int strlen(const char* s);
void str_cpy(char* dest, char* src); 
int str_cmp(char* s1, char* s2);
void printf_0(const char* format);
void printf_1(const char* format, int input);

#endif
