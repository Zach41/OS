#include "headers.h"

PRIVATE char* i2a(int val, int base, char **ps) {
    /* 这里必须用双重指针 */
    int m = val % base;
    int q = val / base;
    if (q) {
	i2a(q, base, ps);
    }
    *(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

    return *ps;
}

PUBLIC int printl(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    buf[len] = 0;
    printx(buf);

    return len;
}

