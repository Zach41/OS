#include "const.h"
#include "type.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

int vsprintf(char* buf, const char* fmt, va_list args) {
    va_list p_next_arg = args;
    
    char  tmp[256];
    memset(tmp, 0, 256);
    char  *str;
    char  *p;
    for (p = buf; *fmt; fmt++) {
	if (*fmt != '%') {
	    *p++ = *fmt;
	    continue;
	}

	fmt++;

	switch(*fmt) {
	case 'x':
	    itoa(tmp, *((int*)p_next_arg));
	    strcpy(p, tmp);
	    p_next_arg += 4;
	    p += strlen(tmp);
	    break;
	case 's':
	    strcpy(p, (*((char**)p_next_arg)));
	    p += strlen(*((char**)p_next_arg));
	    p_next_arg += 4;
	    break;
	case 'd':
	    itod(tmp, *((int*)p_next_arg));
	    strcpy(p, tmp);
	    p_next_arg += 4;
	    p += strlen(tmp);
	default:
	    break;
	}
    }
    return (p - buf);
}

int printf(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    write(buf, len);

    return len;
}

