#include "headers.h"

PUBLIC void assert_failure(char *exp, char *file, char *base_file, int line) {
    printl("%c  assert(%s) failed. file: %s, base_file: %s, ln: %d",
	   MAG_CH_ASSERT,
	   exp,
	   file,
	   base_file,
	   line);

    spin("assertion fialure()");

    __asm__ __volatile__("ud2");
	
}

PUBLIC void spin(char* func_name) {
    printl("\nspining in %s ...\n", func_name);

    while(TRUE) {}
}

PUBLIC void panic(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);

    int len = vsprintf(buf, fmt, args);

    printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

    __asm__ __volatile__("ud2");
}

/* return 0 if same */
PUBLIC int memcmp(const void* s1, const void* s2, int n) {
    if (s1 == 0 || s2 == 0) {
	return (s1 - s2);
    }

    const char* p1 = (const char*)s1;
    const char* p2 = (const char*)s2;

    for (int i=0; i<n; i++) {
	if (*p1++ != *p2++)
	    return 1;
    }
    return 0;
}

PUBLIC void dump_proc(PROCESS* p) {
    printl("Name: %s\nPID:%d\nFLAGS:%x\n", p -> p_name, p -> pid, p -> p_flags);
}
