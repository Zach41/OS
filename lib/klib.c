/* #include "type.h" */
/* #include "const.h" */
/* #include "hd.h" */
/* #include "fs.h" */
/* #include "protect.h" */
/* #include "console.h" */
/* #include "tty.h" */
/* #include "proc.h" */
/* #include "proto.h" */
/* #include "global.h" */
#include "headers.h"

PUBLIC char *itoa(char* str, int num) {
    char *p = str;
    char ch;
    int flag = 0;

    *p++ = '0';
    *p++ = 'x';
    if (num == 0) {
	*p++ = '0';
    } else {
	for (int i=28; i>=0; i-=4) {
	    ch = (num >> i) & 0xF;
	    if (flag || ch > 0) {
		flag = 1;	/* 找到第一个不为0的数 */
		ch += '0';
		if (ch > '9') {
		    ch += 7;	/* ord('9') = 57, ord('A') = 65 */
		}
		*p++ = ch;
	    }
	}
    }
    *p = 0;
    return str;
}

PUBLIC char* itod(char* str, int num) {
    char *p = str;
    int  cnt = 0;

    while (num) {
	*p++ = num % 10 + '0';
	num /= 10;
	cnt += 1;
    }
    *p++ = 0;
    for (int i=1; i <= cnt/2; i++) {
	char ch      = str[i-1];
	str[i-1]     = str[cnt - i];
	str[cnt - i] = ch;
    }

    return str;
     
}

PUBLIC void disp_int(int num) {
    char input[16];
    itoa(input, num);
    disp_str(input);
}

PUBLIC void delay(int time) {
    for (int k=0; k<time; k++) {
	for (int i=0; i<1000; i++) {
	    for (int j=0; j<100; j++) {
		;
	    }
	}
    }
}

/* 更精确的延迟函数 */
/* 延迟milli_sec毫秒 */
PUBLIC void milli_delay(int milli_sec) {
    int ticks = get_ticks();
    while (((get_ticks() - ticks) * 1000 / HZ) < milli_sec) {
	;
    }
}

PUBLIC int strlen(char* str) {
    int cnt = 0;
    char* p = str;
    
    while (*p) {
	cnt++;
	p++;
    }

    return cnt;
}
