#include "type.h"
#include "const.h"
#include "hd.h"
#include "fs.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

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
