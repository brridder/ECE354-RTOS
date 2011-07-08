/*
 * String.c
 */

#include "string.h"
#include "dbug.h"

#define SNPRINTF_BUFFER_SIZE 128
#define FORMAT_INT_BUFFER_SIZE 16

/**
 * Globals used for string formatting
 */
 
char snprintf_buffer[SNPRINTF_BUFFER_SIZE];
char format_int_buffer[FORMAT_INT_BUFFER_SIZE];

/**
 * @brief: Consume a character, return 0 on success, -1 on failure
 * @param: str string to consume from
 * @param: c character to consume
 */

int consume(char** str, const char c) {
  if (**str == c) {
    (*str)++;

    return 0;
  } else {
    return -1;
  }
}

/**
 * @brief: power function: base^exponent
 * @param: base 
 * @param: exponent
 */

int power(int base, int exponent) {
  int i;
  int total;

  if (exponent == 0) {
    return 1;
  } else { 
    total = base;
    for (i = 0; i < exponent - 1; i++) {
      total *= base;
    }

    return total;
  }
}

/**
 * @brief: converts a string into integer, returns -1 if non-digit encountered
 * @param: str input string
 * @param: length number of characters of str to parse
 */ 

int atoi_e(char str[], int length) {
  int i;
  int total;

  total = 0;
  for (i = 0; i < length; i++) {
    if (str[i] < 0x30 || str[i] > 0x39) {
      return -1;
    } else {
      total += (str[i] - 0x30) * power(10, length - i - 1);
    }
  }

  return total;
}


/**
 * @brief: converts integer into c string
 * @param: n input integer
 * @param: s character buffer to write into
 */

void itoa(int n, char s[]) {
    int i, sign;
    if ((sign = n) < 0) {
        n = -n;
    }
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while((n/=10) > 0);
    if (sign < 0) {
        s[i++] = '-';
    }
    s[i] = '\0';
    reverse(s);
}

void itox(unsigned int n, char s[]) {
    int i;
    char digit;

    i = 0;
    do {
        digit = n % 16 + '0';
        if (digit > 57) {
            digit += 7;
        }
        s[i++] = digit;
    } while((n/=16) != 0);
    s[i] = '\0';
    reverse(s);
}

int atoi(const char *s) {
   int res = 0;
   while(*s >= '0' && *s <= '9') {
        res = res * 10  + *s++ - '0';
   }
   return res;
}

void reverse(char s[]) {
    int i,j;
    char c;
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s-str);
}

void str_cpy(char* dest, char* src) {
    while(*dest++ = *src++);
}

int str_cmp(char* s1, char* s2) {
    for( ; *s1 == *s2; ++s1, ++s2) {
        if (*s1 == 0) {
            return 0; // equal
        }
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
/**
 * @brief: Format the input into the string, replacing %i as the input as int.
 * @param: buffer buffer to write to
 * @param: buffer_size maximum length to write to buffer
 * @param: format format string
 * @param: input string to replace 
 */

void snprintf_1(char* buffer, int buffer_size, const char* format, int input) {
    int index;
    const char* format_iter;
    const char* subformat_iter;

    index = 0;
    format_iter = format;
    while (*format_iter) {
        
        //
        // Look for format string combinations
        //
        
        if (*format_iter == '%') {
            format_iter++;

            if (*format_iter == 'i') {
                format_iter++;                

                //
                // Format the input as integer
                //

                itoa(input, format_int_buffer);
                subformat_iter = format_int_buffer;
                while (*subformat_iter) {
                    buffer[index] = *subformat_iter;
                    subformat_iter++;

                    index++;
                    if (index == buffer_size) {
                        goto format_done;
                    }
                }
            } else if (*format_iter == 'x') {
                format_iter++;                

                buffer[index] = '0';
                index++;                
                if (index == buffer_size) {
                    goto format_done;
                }

                buffer[index] = 'x';
                index++;
                if (index == buffer_size) {
                    goto format_done;
                }                                

                //
                // Format the input as hex
                //

                itox(input, format_int_buffer);
                subformat_iter = format_int_buffer;
                while (*subformat_iter) {
                    buffer[index] = *subformat_iter;
                    subformat_iter++;

                    index++;
                    if (index == buffer_size) {
                        goto format_done;
                    }
                }
            } else if (*format_iter == '%') {

                //
                // Literal %
                //
                
                buffer[index] = '%';

                index++;
                if (index == buffer_size) {
                    goto format_done;
                }
            }
        } else {

            //
            // Copy the format string character to the buffer
            //
            
            buffer[index] = *format_iter;
            format_iter++;

            index++;
            if (index == buffer_size) {
                goto format_done;
            }
        }
    }

    buffer[index] = '\0';

 format_done:
    return;
}

/**
 * @brief: print a formatted string to JanusROM terminal
 * @param: format format string
 * @param: input input to subsitute into the format string
 */

void printf_1(const char* format, int input) {
    snprintf_1(snprintf_buffer, SNPRINTF_BUFFER_SIZE, format, input);
    rtx_dbug_outs(snprintf_buffer);
}

/**
 * @brief: print a formatted string to JanusROM terminal
 */

void printf_0(const char* format) {
    printf_1(format, 0);
}
