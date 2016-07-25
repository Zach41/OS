#include "headers.h"
#include "stdio.h"

PUBLIC int printf(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    buf[len] = 0;
    int cnt = write(FD_STDOUT, buf, len);
    assert(cnt == len);

    return cnt;
}
