/*
 * String.c
 */

#include "string.h"

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

void reverse(s) {
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
