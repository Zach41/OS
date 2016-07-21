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

PUBLIC int vsprintf(char* buf, const char* fmt, va_list args) {
    va_list p_next_arg = args;
    
    char  inner_buf[256];
    char  *p;
    int   m;
    int   align_nr;
    char  cs;
    for (p = buf; *fmt; fmt++) {
	if (*fmt != '%') {
	    *p++ = *fmt;
	    continue;
	} else {
	    align_nr = 0;
	}

	fmt++;

	if (*fmt == '%') {
	    *p++ = *fmt++;
	    continue;
	} else if (*fmt == '0'){
	    cs = '0';
	    fmt++;
	} else {
	    cs = ' ';
	}

	while (((unsigned char)(*fmt) >= '0') && ((unsigned char)(*fmt) <= '9')) {
	    align_nr *= 10;
	    align_nr += *fmt - '0';
	    fmt++;
	}

	char *q = inner_buf;
	memset(q, 0, sizeof(inner_buf));
	
	switch(*fmt) {
	case 'c':
	    *q++ = *((char*)p_next_arg);
	    p_next_arg += 4;
	    break;
	case 'x':
	    m = *((int*)p_next_arg);
	    i2a(m, 16, &q);
	    p_next_arg += 4;
	    break;
	case 'd':
	    m = *((int*)p_next_arg);
	    if (m < 0) {
		m = m * (-1);
		*q++ = '-';
	    }
	    i2a(m, 10, &q);
	    p_next_arg += 4;
	    break;
	case 's':
	    strcpy(q, (*((char**)p_next_arg)));
	    q += strlen(*((char**)p_next_arg));
	    p_next_arg += 4;
	    break;
	default:
	    break;
	}

	for (int k=0; k < (align_nr > strlen(inner_buf) ? (align_nr - strlen(inner_buf)) : 0); k++) {
	    *p++ = cs;
	}
	q = inner_buf;
	while(*q) {
	    *p++ = *q++;
	}
    }
    *p = 0;
    return (p - buf);
}

int printf(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    buf[len] = 0;
    write(buf, len);

    return len;
}

PUBLIC int printl(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    buf[len] = 0;
    printx(buf);

    return len;
}

PUBLIC int sprintf(char* buf, const char* fmt, ...) {
    va_list args = (va_list)((char*)(&fmt) + 4);

    return vsprintf(buf, fmt, args);
}
